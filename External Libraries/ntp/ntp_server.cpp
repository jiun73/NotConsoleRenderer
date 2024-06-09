#if defined(_MSC_VER) && !defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ws2tcpip.h>
#endif

#ifndef WIN32//linux/unix
#include <sys/socket.h>
#include <arpa/inet.h>
#endif//WIN32

#include "ntp_server.h"
#include "utils.h"
#include "sockets_helper.h"
#include <assert.h>
#include <sstream>
#include <thread>
#include <functional>

namespace ntp {
	struct client_req {
		NtpPacket::NtpTimeFormat arrive_time;
		NtpPacketBuffer in_ntp_msg;
		union
		{
			sockaddr_in client_addr_ipv4;
			sockaddr_in6 client_addr_ipv6;
		}
		client_addr;
		sockaddr* get_sockaddr() { return (sockaddr*)&client_addr; }
		const sockaddr* get_sockaddr() const { return (const sockaddr*)&client_addr; }
		std::pair<ManagedClockPtr, ServerInfoPtr> clock_info;
	};
}

using namespace ntp;

static std::string AddressToString(const sockaddr_in& addr) {
	char ipStr[INET_ADDRSTRLEN];
	if (inet_ntop(addr.sin_family, &addr.sin_addr, ipStr, sizeof(ipStr)) != nullptr)
		return "[" + std::string(ipStr) + "]";

	return "[unknown]";
}

static std::string AddressToString(const sockaddr_in6& addr) {
	char ipStr[INET6_ADDRSTRLEN];
	if (inet_ntop(addr.sin6_family, &addr.sin6_addr, ipStr, sizeof(ipStr)) != nullptr)
		return "[" + std::string(ipStr) + "]";

	return "[unknown]";
}

static std::string AddressToString(const client_req& client_req) {
	if (AF_INET == client_req.get_sockaddr()->sa_family)
		return AddressToString(client_req.client_addr.client_addr_ipv4);
	else
		return AddressToString(client_req.client_addr.client_addr_ipv6);
}


std::shared_ptr<ntp::IServer> ntp::IServer::create_instance() {
	return std::make_shared<ntp::Server>();
}

ntp::Server::Server() :
	m_p_si_addr(nullptr)
	, m_socket(ntp::sockets::invalid_socket)
	, m_p_si_addr_size(0)
	, m_address_family(AF_UNSPEC)
	, m_disposing_evt(false), m_requestsQueueSem(semaphore::MAX_COUNT, 0)
	, m_clientsRespondedPostProcessQueueSem(semaphore::MAX_COUNT, 0)
	, m_flaged_asout_of_synch(false)
{
	ntp::init();
	ntp::utils::switch_os_accurate_clock(true);
}

void ntp::Server::cleanup_listening_socket()
{
	if (ntp::sockets::invalid_socket != m_socket) {//release the current socket
		ntp::sockets::shutdown_socket(m_socket, ntp::sockets::shutdown_what::both);
		ntp::sockets::close_socket(m_socket);
		m_socket = ntp::sockets::invalid_socket;
	}

	if (m_clients_request_listening_thread.joinable())
		m_clients_request_listening_thread.join();

	if (m_p_si_addr) {//release the current address mem
		free(m_p_si_addr);
		m_p_si_addr = nullptr;
	}

	m_p_si_addr_size = 0;
	m_address_family = AF_UNSPEC;
}

ntp::Server::~Server()
{
	m_disposing_evt.set();

	if (m_clients_response_thread.joinable()) {
		m_requestsQueueSem.releaseOne();
		m_clients_response_thread.join();
	}

	if (m_clients_post_process_thread.joinable()) {
		m_clientsRespondedPostProcessQueueSem.releaseOne();
		m_clients_post_process_thread.join();
	}

	if (m_clock_update_thread.joinable())
		m_clock_update_thread.join();

	cleanup_listening_socket();
	ntp::utils::switch_os_accurate_clock(false);
}

bool ntp::Server::init(const ServerSettings & settings, const EndPointUDP & listen_ep)
{
	m_settings = settings;
	if (m_settings.time_source && m_settings.time_source->has_time())
		set_clock(m_settings.time_source->clock(), m_settings.time_source->active_server().first);
	else
		set_clock(ManagedClockPtr(new ManagedClock(std::chrono::system_clock::now(), std::chrono::steady_clock::now(), 0.0)));

	if (!ntp::sockets::is_socket_lib_initialized())
	{
		m_lastErrorMsg = "ntp::Server::init failed on sockets init";

		if (m_settings.error_callback)
			m_settings.error_callback(*this, m_lastErrorMsg, m_settings.userParam);

		return false;
	}

	cleanup_listening_socket();

	//attempt to classify the IP (v4/v6) & setup address structure
	{
		sockaddr_in addr_v4{ 0 }; addr_v4.sin_family = AF_INET; addr_v4.sin_port = htons(listen_ep.port);
		sockaddr_in6 addr_v6{ 0 }; addr_v6.sin6_family = AF_INET6; addr_v6.sin6_port = htons(listen_ep.port);
		if (1 == inet_pton(addr_v4.sin_family, listen_ep.host.c_str(), &addr_v4.sin_addr)) {
			m_p_si_addr_size = sizeof(addr_v4);
			m_p_si_addr = malloc(m_p_si_addr_size);
			memcpy(m_p_si_addr, &addr_v4, m_p_si_addr_size);
			m_address_family = addr_v4.sin_family;
		}
		else if (1 == inet_pton(addr_v6.sin6_family, listen_ep.host.c_str(), &addr_v6.sin6_addr)) {
			m_p_si_addr_size = sizeof(addr_v6);
			m_p_si_addr = malloc(m_p_si_addr_size);
			memcpy(m_p_si_addr, &addr_v6, m_p_si_addr_size);
			m_address_family = addr_v6.sin6_family;
		}
		else {
			std::ostringstream oss; oss << "ntp::Server::init failed resolving the endpoint: " << listen_ep.host;
			m_lastErrorMsg = oss.str();
			if (m_settings.error_callback)
				m_settings.error_callback(*this, m_lastErrorMsg, m_settings.userParam);

			return false;
		}
	}

	//create socket	
	if ((m_socket = socket(m_address_family, SOCK_DGRAM, IPPROTO_UDP)) == ntp::sockets::socket_error) {
		std::stringstream oss; oss << "ntp::Server::init failed creating a socket: " << ntp::sockets::SocketError::get_last_error().msg;
		m_lastErrorMsg = oss.str();
		cleanup_listening_socket();
		if (m_settings.error_callback)
			m_settings.error_callback(*this, m_lastErrorMsg, m_settings.userParam);

		return false;
	}

	if (!m_settings.exclusive_address_usage)
	{
		if (ntp::sockets::socket_error == ntp::sockets::set_socket_shared_listening_addr_port(m_socket)) {
			std::stringstream oss; oss << "ntp::Server::init failed setting [exclusive_address_usage] socket option: " << ntp::sockets::SocketError::get_last_error().msg << ", ignoring.";
			m_lastErrorMsg = oss.str();//Do not return (exclusive_address_usage flag is not set but socket should will still work).
			if (m_settings.warnning_callback)
				m_settings.warnning_callback(*this, m_lastErrorMsg, m_settings.userParam);
		}
	}

	if (0 != bind(m_socket, (sockaddr*)m_p_si_addr, m_p_si_addr_size))
	{
		std::ostringstream oss; oss << "ntp::Server::init binding socket: " << ntp::sockets::SocketError::get_last_error().msg;
		m_lastErrorMsg = oss.str();
		cleanup_listening_socket();
		if (m_settings.error_callback)
			m_settings.error_callback(*this, m_lastErrorMsg, m_settings.userParam);

		return false;
	}

	m_clients_response_thread = std::thread(std::bind(&Server::clients_response_work, this));
	m_clients_request_listening_thread = std::thread(std::bind(&Server::clients_request_listening_work, this));
	m_clients_post_process_thread = std::thread(std::bind(&Server::clients_responded_post_process_work, this));

	if (m_settings.time_source)
		m_clock_update_thread = std::thread(std::bind(&Server::clock_update_work, this));

	return true;
}

ManagedClockPtr ntp::Server::inner_clock() const { return clock().first; }

void ntp::Server::flag_as_out_of_synch(bool out_of_synch) { m_flaged_asout_of_synch = out_of_synch; }

void ntp::Server::clients_response_work()
{
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#else //unix/linux
	//Set thread priority(Thread Scheduling Policy : SCHED_FIFO)
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif//WIN32

	//Server_precision: express the ability to maintain stable clock over short period of time.
	const auto server_precision = calculate_precision(std::chrono::microseconds(1));

	std::shared_ptr<client_req> poped_req;
	NtpPacketBuffer send_packet;
	while (true)
	{
		memset(&send_packet, 0, sizeof(send_packet));
		m_requestsQueueSem.wait();
		{
			std::lock_guard<std::mutex> lock(m_requestsQueueLock);
			if (m_requestsQueue.empty()) break;//flag to close the thread
			poped_req = m_requestsQueue.front();
			m_requestsQueue.pop();
		}

		if (AssociationModes::Client != get_ntp_mode(poped_req->in_ntp_msg.pkt.li_vn_mode)) {
			//the sender is not configured as client on the packet.
			if (m_settings.warnning_callback) {
				std::ostringstream oss;
				oss << "ntp::Server client " << AddressToString(*poped_req) << " request ignored: ";
				oss << "Wrong AssociationModes privided : " << static_cast<int>(get_ntp_mode(poped_req->in_ntp_msg.pkt.li_vn_mode));
				m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);
			}
			continue;
		}

		ntp::ProtocolVersion clients_protocol = get_ntp_version(poped_req->in_ntp_msg.pkt.li_vn_mode);
		bool report_out_of_synch = m_flaged_asout_of_synch;

		switch (clients_protocol)
		{
		case ntp::ProtocolVersion::V3:
			//NTP V3 do not support IPV6, there  are several options:
			//1. Ignore the client request entirely.
			//2. Service the client but treat the request as NTP v4 request.
			//3. Service the client but treat the request as NTP v4 request, but put NTP v3 on the respsonse in the protocol field.
			//4. respose with error code.
#if 1
			if (AF_INET6 == m_address_family)
			{
				std::ostringstream oss;
				oss << "ntp::Server client " << AddressToString(*poped_req) << " request ignored: ";
				oss << "Requested NTP V3, respose while communicating on IPv6.";
				m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);
				report_out_of_synch = true;
			}
#else		
			clients_protocol = ntp::ProtocolVersion::V4;
#endif
			break;
		case ntp::ProtocolVersion::V4:
			break;
		default:
			//the sender requested unsupported protocol version.
			if (m_settings.warnning_callback) {
				std::ostringstream oss;
				oss << "ntp::Server client " << AddressToString(*poped_req) << " request ignored: ";
				oss << "Unsupported protocol version requested.";
				m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);
			}
			continue;
		}

		auto LI = LeapIndicator::NoWarning;
		//When server's leap indiactor info is available, make it public
		if (poped_req->clock_info.second && 
				(
					LeapIndicator::LastMinuteOfTheDayLeapSecondInsertion == poped_req->clock_info.second->leap_indicator ||
					LeapIndicator::LeapSecondInserted == poped_req->clock_info.second->leap_indicator
				)
			)
		{
			LI = poped_req->clock_info.second->leap_indicator;
		}

		/**
		The NTPv4 server should behave as follows when it receives an NTPv3 packet :
		If the NTPv3 packet contains a valid timestamp, the NTPv4 server should respond with an NTPv4 packet.
		If the NTPv3 packet contains an invalid timestamp, the NTPv4 server should discard the packet and not respond.

		In the future when support will be added to authenticated packets in v4:
		If the packet is not 64 bytes long, but its 48 bytes long, treat it as unauthenticated packets,
		is not, the server should drops the request.
		Assuming that the received message size is 68 bytes, the server extracts the RID from the received message.*/

		/* filling the "LI VN Mode" keeping the client's version
			LI   = 0
			VN   = Version number at client.
			Mode = 4 //NTP_MODE_SERVER */

		//Since excet for the authentication part ntp v3 & v4 are identical
		//we will use the same protocol version the client requestd.
		send_packet.pkt.li_vn_mode = create_li_vn_mode(ntp::AssociationModes::Server, 
													   report_out_of_synch ? LeapIndicator::NotSynched : LI,
													   clients_protocol
													   );

		//If we are have reference to external server, use it's info:
		if (poped_req->clock_info.second && !m_settings.report_minimal_root_dispersion_delay)
		{
			std::chrono::nanoseconds root_delay, root_dispersion;
			calc_root_values(poped_req->clock_info.second, root_delay, root_dispersion);
			ntp::get_ntp_duration_from_chrono_duration<uint16_t>(send_packet.pkt.rootDelay.segments.s, send_packet.pkt.rootDelay.segments.f, root_delay);
			ntp::get_ntp_duration_from_chrono_duration<uint16_t>(send_packet.pkt.rootDispersion.segments.s, send_packet.pkt.rootDispersion.segments.f, root_dispersion);
		}
		else {//We do not have info in reference to external server, or the user settings set to fabricate something.
			//falsely report minimal rootDispersion & rootDelay values.
			send_packet.pkt.rootDispersion.all = send_packet.pkt.rootDelay.all = 0;//use minimal values
		}

		const StratumLevel reportedStratum = poped_req->clock_info.second ?
			std::min(StratumLevel::SecondaryReference15, StratumLevel((uint8_t)poped_req->clock_info.second->stratum.value + 1)) :
			StratumLevel::SecondaryReference13;

		send_packet.pkt.stratum = static_cast<uint8_t>(reportedStratum);

		send_packet.pkt.rootDispersion.segments.s = htons(send_packet.pkt.rootDispersion.segments.s);
		send_packet.pkt.rootDispersion.segments.f = htons(send_packet.pkt.rootDispersion.segments.f);

		send_packet.pkt.rootDelay.segments.s = htons(send_packet.pkt.rootDelay.segments.s);
		send_packet.pkt.rootDelay.segments.f = htons(send_packet.pkt.rootDelay.segments.f);

		/* Set Precision*/
		send_packet.pkt.precision = server_precision;

		/* Reference ID (4 characters/bytes)= */
		//It should NOT be convert to network order.
		send_packet.pkt.refId = calc_server_refid(clients_protocol, reportedStratum, poped_req->clock_info.second);

		/* Copy Poll from the client. if the client calls are too rapid, consider using calculate_clients_polling_min_intervals() */
		send_packet.pkt.poll = poped_req->in_ntp_msg.pkt.poll;

		//setting the reference timestamp.
		//The reference timestamp (64 bits) is the timestamp taken from a reliable time source that the NTP server uses as a reference to synchronize its own clock.
		//It represents the time at which the server's clock was last set or corrected.
		//Normally if the current server it self is getting its clock from another time source or server(upstream server)
		//value should be the transmit timestamp on the NTP packet receved by the upstream server, or in general the time that the 
		//clock used was last updated/corrected
		poped_req->clock_info.first->reference_time().as_ntp_time(send_packet.pkt.refTm.segments.s, send_packet.pkt.refTm.segments.f);
		send_packet.pkt.refTm.segments.s = htonl(send_packet.pkt.refTm.segments.s);
		send_packet.pkt.refTm.segments.f = htonl(send_packet.pkt.refTm.segments.f);

		// Set the originate timestamp to the client's transmit timestamp
		send_packet.pkt.origTm = poped_req->in_ntp_msg.pkt.txTm;

		//Time when the message as received at the sever
		send_packet.pkt.rxTm.segments.s = htonl(poped_req->arrive_time.segments.s);
		send_packet.pkt.rxTm.segments.f = htonl(poped_req->arrive_time.segments.f);

		//Setting transmission time (Transmit Time)
		poped_req->clock_info.first->now().as_ntp_time(send_packet.pkt.txTm.segments.s, send_packet.pkt.txTm.segments.f);
		send_packet.pkt.txTm.segments.s = htonl(send_packet.pkt.txTm.segments.s);
		send_packet.pkt.txTm.segments.f = htonl(send_packet.pkt.txTm.segments.f);

		const int send_res = ntp::sockets::sendto(m_socket, send_packet.as_buffer, NTP_PACKET_LEN, 0, poped_req->get_sockaddr(), m_p_si_addr_size);
		if (NTP_PACKET_LEN != send_res)
		{
			if (ntp::sockets::socket_error == send_res)
			{
				std::ostringstream oss; oss << "ntp::Server client " << AddressToString(*poped_req) << " responding failed: " << ntp::sockets::SocketError::get_last_error().msg;
				m_lastErrorMsg = oss.str();
				if (m_settings.warnning_callback)
					m_settings.warnning_callback(*this, m_lastErrorMsg, m_settings.userParam);

				assert(0);
			}
			continue;
		}
		else
		{
			std::lock_guard<std::mutex> lock(m_clientsRespondedPostProcessQueueLock);
			m_clientsRespondedPostProcessQueue.push(poped_req);
			m_clientsRespondedPostProcessQueueSem.releaseOne();
		}

	}
}


void ntp::Server::clients_request_listening_work()
{
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#else //unix/linux
	//Set thread priority(Thread Scheduling Policy : SCHED_FIFO)
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif//WIN32

	client_req req;
	while (true)
	{
		int client_addr_size = m_p_si_addr_size;
		int ret = ntp::sockets::recvfrom(m_socket, req.in_ntp_msg.as_buffer, NTP_PACKET_LEN, 0, req.get_sockaddr(), &m_p_si_addr_size);

		req.clock_info = clock();//using the same clock instanse throught the request handling.
		req.clock_info.first->now().as_ntp_time(req.arrive_time.segments.s, req.arrive_time.segments.f);
		if (0 == ret)
			break;//connection has been gracefully closed,

		if (ntp::sockets::socket_error == ret)
		{
			const auto last_err = ntp::sockets::SocketError::get_last_error();
			//socket shut down and disposed before listening ?
			const bool sock_disposed_before_recvfrom = m_disposing_evt.wait(0) && last_err.is_one_of({ ENOTSOCK, EBADF });

			//socket shut down while listening ?
			const bool interrup_io_unblock = last_err.is(EINTR);
			const bool sock_shutdown = last_err.is(ENOTCONN);
			if (!sock_shutdown && !interrup_io_unblock && !sock_disposed_before_recvfrom)
			{
				std::stringstream oss; oss << "ntp::Server client listening failed: " << last_err.msg;
				m_lastErrorMsg = oss.str();

				if (m_settings.error_callback)
					m_settings.error_callback(*this, m_lastErrorMsg, m_settings.userParam);

				//Another listen error that can take place is ECONNRESET (an existing connection was forcibly closed by the remote host).
				//Its may invoke if in ECONNRESET middle of listening, the sattus of the network adpater changed
				if (last_err.is(ECONNRESET))continue;//continue try listening
				assert(0);
			}

			break;
		}

		if (NTP_PACKET_LEN != ret) //in the future support ntpv4 packets with authentication.
		{
			std::stringstream oss; oss << "ntp::Server client " << AddressToString(req) << " sent wrong packet size, ignoring.";
			m_lastErrorMsg = oss.str();
			if (m_settings.warnning_callback)
				m_settings.warnning_callback(*this, m_lastErrorMsg, m_settings.userParam);

			assert(0);
			continue;//invalid packet size;
		}

		{
			std::lock_guard<std::mutex> lock(m_requestsQueueLock);
			m_requestsQueue.push(std::shared_ptr<client_req>(new client_req(req)));
		}
		m_requestsQueueSem.releaseOne();

	}//end of while loop;
}

void ntp::Server::clients_responded_post_process_work()
{
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#else //unix/linux
	//Set thread priority(Thread Scheduling Policy : SCHED_FIFO)
	// Set scheduling policy to SCHED_OTHER (equivalent to THREAD_PRIORITY_BELOW_NORMAL)
	struct sched_param params;
	params.sched_priority = 0; // Not used for SCHED_OTHER
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &params);
#endif//WIN32

	std::shared_ptr<client_req> poped_req;
	while (true)
	{
		m_clientsRespondedPostProcessQueueSem.wait();
		{
			std::lock_guard<std::mutex> lock(m_clientsRespondedPostProcessQueueLock);
			if (m_clientsRespondedPostProcessQueue.empty()) break;//flag to close the thread
			poped_req = m_clientsRespondedPostProcessQueue.front();
			m_clientsRespondedPostProcessQueue.pop();
		}

		if (m_settings.client_served_callback)
			m_settings.client_served_callback(*this, AddressToString(*poped_req), m_settings.userParam);

	}//end of while loop;
}

void ntp::Server::clock_update_work()
{
	while (!m_disposing_evt.wait(1000 * 5))
	{
		if (m_settings.time_source->has_time())
			set_clock(m_settings.time_source->clock(), m_settings.time_source->active_server().first);
	}
}

NtpPacket::ReferenceIdentifier ntp::Server::calc_server_refid(ntp::ProtocolVersion ver, ntp::StratumLevel stratum, ntp::ServerInfoPtr reference_server) const
{
	NtpPacket::ReferenceIdentifier refid;
	memset(&refid, 0, sizeof(refid));

	if (!reference_server) {
		//This server is not connected to external server (its using its own local clock), in this special case,
		//we will state that the refid is four-character ASCII “LOCL”,
		//it means that the NTP server is using its own local clock as a reference clock
		memcpy(refid.ascii, "LOCL", sizeof(refid));
		return refid;
	}

	// In the case of NTP Version 3 or Version 4 stratum-0 (unspecified)
	// or stratum-1 (primary) servers, this is a four-character ASCII
	// string, left justified and zero padded to 32 bits (our server will never be stratum-0).
	if (StratumLevel::Unspecified == stratum || StratumLevel::PrimaryReference == stratum)
	{
		memcpy(refid.ascii, "NTPS", sizeof(refid));//our one code...
		return refid;
	}

	//At this point, its a secondary server (stratum > 1), the refid should refer to the IP of the reference clock.
	//Normally on IPv4 it will be the 4 bytes of the IP,
	//However on IPv16 (16 bytes that cannot be stored of refid) it should be the 1st 4 bytes of MD5 output, executed on the 16 bytes of the IPv6 

	if (ntp::IPAddress::Version::V4 == reference_server->server_ip->ver)
		memcpy(refid.bytes, reference_server->server_ip->addr.v4, sizeof(refid));
	else
	{
		//psudo code:
		//memcpy(refid.bytes, md5(reference_server->server_ip->addr.v6), sizeof(refid));

		//Instead of calcualting MD5 on reference_server->server_ip->addr.v6, that no one will ever use,
		//we will keep it empty string, so that at least the client will not treat it as IP.
	}

	return refid;
}

void ntp::Server::check_root_values(ServerInfoPtr server)
{
	std::chrono::nanoseconds root_delay, root_dispersion;
	calc_root_values(m_clock_src_info, root_delay, root_dispersion);
	if (std::chrono::milliseconds(1000) <= std::max(root_dispersion, root_delay) && m_settings.warnning_callback) {
		std::ostringstream oss;
		if (std::chrono::milliseconds(1000) <= root_delay)
			oss << "Server clock's root_delay is high (" << std::chrono::duration_cast<ntp::milliseconds_fp>(root_delay).count() << " millisec), some client will reject the server's clock." << std::endl;
		if (std::chrono::milliseconds(1000) <= root_dispersion)
			oss << "Server clock's root_dispersion is high (" << std::chrono::duration_cast<ntp::milliseconds_fp>(root_dispersion).count() << " millisec), some client will reject the server's clock." << std::endl;

		oss << "Consider replacing clock source or turning on the [report_minimal_root_dispersion_delay] option";
		m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);
	}
}
