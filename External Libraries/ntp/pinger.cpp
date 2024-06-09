
#include "pinger.h"
#include <thread>
#include <random>
#include <assert.h>
#include "sockets_helper.h"
#include <cstring>//for std::memset/memcpy
#if defined(_MSC_VER) && !defined(WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else//linux/unix
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	//#include <sys/capability.h>//required "sudo apt-get install libcap-dev"
#endif

// The following two structures need to be packed tightly.
// The IP header
struct iphdr {
	uint8_t h_len : 4;          // Length of the header in dwords
	uint8_t version : 4;        // Version of IP
	uint8_t tos;               // Type of service
	uint16_t total_len;       // Length of the packet in dwords
	uint16_t ident;           // unique identifier
	uint16_t flags;           // Flags
	uint8_t ttl;               // Time to live
	uint8_t proto;             // Protocol number (TCP, UDP etc)
	uint16_t checksum;        // IP checksum
	uint32_t source_ip;
	uint32_t dest_ip;
};

//struct in6_addr {
//	union {
//		uint8_t u6_addr8[16];
//		uint16_t u6_addr16[8];
//		uint32_t u6_addr32[4];
//	} in6_u;
//#define s6_addr                 in6_u.u6_addr8
//#define s6_addr16               in6_u.u6_addr16
//#define s6_addr32               in6_u.u6_addr32
//};

struct ipv6hdr {
	uint8_t  priority : 4;
	uint8_t	 version : 4;
	uint8_t  flow_lbl[3][4];
	uint16_t total_len;
	uint8_t  nexthdr;
	uint8_t  hop_limit;
	struct  in6_addr saddr;
	struct  in6_addr daddr;
};

// ICMP header
struct icmphdr {
	uint8_t type;          // ICMP packet type
	uint8_t code;          // Type sub code
	uint16_t checksum;

	union {
		struct {
			uint16_t id;
			uint16_t sequence;
		}echo;
		uint32_t gateway;

		struct {
			uint16_t __unused;
			uint16_t mtu;

		}frag;
	}un;

	uint32_t timestamp;    // not part of ICMP, but we need it
};

struct icmp6_hdr {
	uint8_t icmp6_type;   // ICMPv6 packet type
	uint8_t icmp6_code;   // Type sub code
	uint16_t icmp6_cksum; // Checksum
	union {
		uint32_t icmp6_un_data32[1]; // Type-specific field
		uint16_t icmp6_un_data16[2]; // Type-specific field
		uint8_t icmp6_un_data8[4];   // Type-specific field
	} icmp6_dataun;
};

namespace ipv4 {
	enum icmp : uint8_t
	{
		//ICMP packet types:
		echoreply = 0,
		dest_unreach = 3,
		time_expire = 11,
		echo = 8,////echo request:
	};
}
namespace ipv6 {
	enum icmp
	{
		//ICMP packet types:
		echoreply = 129,
		dest_unreach = 1,
		time_expire = 3,
		echo = 128,//echo request:
	};
}

static std::string getDestinationUnreachableMessage(int code) {
	switch (code) {
	case 0:
		return "Network Unreachable";
	case 1:
		return "Host Unreachable";
	case 2:
		return "Protocol Unreachable";
	case 3:
		return "Port Unreachable";
	case 4:
		return "Fragmentation Needed and Don't Fragment was Set";
	case 5:
		return "Source Route Failed";
	case 6:
		return "Destination Network Unknown";
	case 7:
		return "Destination Host Unknown";
	case 8:
		return "Source Host Isolated";
	case 9:
		return "Communication with Destination Network is Administratively Prohibited";
	case 10:
		return "Communication with Destination Host is Administratively Prohibited";
	case 11:
		return "Destination Network Unreachable for Type of Service";
	case 12:
		return "Destination Host Unreachable for Type of Service";
	case 13:
		return "Communication Administratively Prohibited by Filtering";
	default:
		return "Unknown Destination Unreachable Code";
	}
}

// Calculate ICMP checksum
static unsigned short checksum(void *b, int len) {
	unsigned short *buf = reinterpret_cast<unsigned short *>(b);
	unsigned int sum = 0;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;

	if (len == 1)
		sum += *(unsigned char *)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return static_cast<unsigned short>(~sum);
}


Pinger::Pinger() : m_sockfd(ntp::sockets::invalid_socket), m_af_net(AF_UNSPEC) {
	ntp::sockets::socket_lib_init();
}

Pinger::~Pinger() { cleanup(); }

void Pinger::cleanup()
{
	m_af_net = AF_UNSPEC;
	if (ntp::sockets::invalid_socket != m_sockfd) {
		ntp::sockets::close_socket(m_sockfd);
		m_sockfd = ntp::sockets::invalid_socket;
	}
}


bool Pinger::HasSufficiantPrivileges()
{
#ifdef WIN32
	return true;
#else
	//in linux, to use raw sockets, one should be eather root user, or has the CAP_NET_RAW capability set.
	if (0 == geteuid()) return true; //Root privileges detected.
	return false;

//	bool has_net_raw_capability = false;
//
//	auto caps = cap_get_proc();
//	if(caps)
//    {
//        cap_flag_value_t cap_value;
//        if (0== cap_get_flag(caps, CAP_NET_RAW, CAP_EFFECTIVE, &cap_value)){
//            if(CAP_SET ==  cap_value) has_net_raw_capability = true;//CAP_NET_RAW capability is set
//            cap_free(caps);
//        }
//	}
//	return has_net_raw_capability;
#endif // WIN32
}

bool Pinger::Init(std::string target_host, int ttl, std::chrono::milliseconds timeout)
{
    if(!HasSufficiantPrivileges())
        return false;

	m_timeout = timeout;
	m_ttl = ttl;
	cleanup();

	ntp::sockets::addr_type addr_type;
	m_target_ip_str = ntp::sockets::host_name_to_ip(target_host, addr_type);
	if (m_target_ip_str.empty()) {
		cleanup();
		return false;
	}
	switch (addr_type)
	{
	case ntp::sockets::addr_type::ipv4:
		m_af_net = AF_INET;
		m_sockfd = socket(m_af_net, SOCK_RAW, IPPROTO_ICMP);
		break;
	case ntp::sockets::addr_type::ipv6:
		m_af_net = AF_INET6;
		m_sockfd = socket(m_af_net, SOCK_RAW, IPPROTO_ICMPV6);
		break;
	default:
		assert(0);
		throw std::exception();
	}

	if (ntp::sockets::invalid_socket == m_sockfd) {
		cleanup();
		return false;
	}

	if (ttl > 255) ttl = 255;

	if (ntp::sockets::set_socket_recv_timeout(m_sockfd, m_timeout) == ntp::sockets::socket_error) {
		cleanup();
		return false;
	}

	if (setsockopt(m_sockfd, IPPROTO_IP, IP_TTL, (const char*)&m_ttl, sizeof(m_ttl)) == ntp::sockets::socket_error) {
		cleanup();
		return false;
	}
	return true;

}

template <typename T>
T generateRandomKey() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<T> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
	return dis(gen);
}

std::vector<Pinger::Result> Pinger::Start(uint16_t nb_requests, std::chrono::nanoseconds requests_intervals)
{
	std::vector<Result> res;
	for (uint16_t i = 0; i < nb_requests; ++i)
	{
		PingRequest pr;	Result requestResult;
		if (!send_ping_request(m_target_ip_str, i + 1, pr)) {
			res.push_back(requestResult);
			continue;
		}

		bool failed = false;
		//sinch this is raw sockets, other icmp packets that will be incoming in pararllel to the one we sent.
		//may be "lifted" by our sockets. Therefor if packet arrived form unexpected source we will continue to wait for the replay:
		while (!failed && !wait_For_ping_reply(i + 1, pr))
		{
			if (pr.receiving_from_wrong_source)
			{
				if ((m_timeout + std::chrono::milliseconds(50)) <= std::chrono::steady_clock::now() - pr.tx)
				{
					requestResult.err_msg = "recv timedout";
					failed = true;
				}
				continue;
			}

			requestResult.err_msg = pr.err_msg;
			failed = true;
		}

		requestResult.round_trip = pr.rx - pr.tx;
		requestResult.success = !failed;
		requestResult.ttl = pr.receivedTTL;
		int sent_ttl = 128 /*m_ttl*/;//patch
		requestResult.approximate_routers_crossed = sent_ttl - pr.receivedTTL + 1; // Add 1 to include the destination router
		res.push_back(requestResult);

		if (0 < requests_intervals.count() && i < nb_requests - 1)
			std::this_thread::sleep_for(requests_intervals);
	}

	return res;
}
bool Pinger::send_ping_request(std::string destinationIp, uint16_t sequenceNumber, PingRequest &request)
{
	switch (m_af_net)
	{
	case AF_INET:
		return send_ping_request_ipv4(destinationIp, sequenceNumber, request);
	case AF_INET6:
		return send_ping_request_ipv6(destinationIp, sequenceNumber, request);
	default:
		assert(0);
		throw std::exception();
	}
}

bool Pinger::send_ping_request_ipv4(std::string destinationIp, uint16_t sequenceNumber, PingRequest & request)
{
	request.sequenceNumber = sequenceNumber;

	sockaddr_in destAddr{ 0 };
	const int destAddr_len = sizeof(destAddr);
	destAddr.sin_family = m_af_net;
	if (1 != inet_pton(destAddr.sin_family, destinationIp.c_str(), &destAddr.sin_addr))
		return false;

	memcpy(request.target_ip, &destAddr.sin_addr, sizeof(destAddr.sin_addr));
	request.id = generateRandomKey<uint16_t>();

	// Set ICMP header fields
	icmphdr icmpHeader{ 0 };
	icmpHeader.type = ipv4::echo;
	icmpHeader.code = 0;
	icmpHeader.checksum = 0;//set to to before calculating checksum
	icmpHeader.un.echo.id = htons(request.id);
	icmpHeader.un.echo.sequence = htons(sequenceNumber);
	icmpHeader.checksum = checksum(&icmpHeader, sizeof(icmphdr));

	request.sequenceNumber = sequenceNumber;
	request.tx = std::chrono::steady_clock::now();

	return ntp::sockets::socket_error != ntp::sockets::sendto(m_sockfd, (const char*)&icmpHeader, sizeof(icmpHeader), 0, (sockaddr*)&destAddr, destAddr_len);
}

bool Pinger::send_ping_request_ipv6(std::string destinationIp, uint16_t sequenceNumber, PingRequest & request)
{
	request.sequenceNumber = sequenceNumber;

	sockaddr_in6 destAddr{ 0 };
	const int destAddr_len = sizeof(destAddr);
	destAddr.sin6_family = m_af_net;
	if (1 != inet_pton(destAddr.sin6_family, destinationIp.c_str(), &destAddr.sin6_addr))
		return false;

	memcpy(request.target_ip, &destAddr.sin6_addr, sizeof(destAddr.sin6_addr));
	request.id = generateRandomKey<uint16_t>();

	// Set ICMP header fields
	icmp6_hdr icmpHeader{ 0 };
	icmpHeader.icmp6_type = ipv6::echo;
	icmpHeader.icmp6_code = 0;
	icmpHeader.icmp6_cksum = 0;//set to to before calculating checksum
	icmpHeader.icmp6_dataun.icmp6_un_data16[0] = htons(request.id);
	icmpHeader.icmp6_dataun.icmp6_un_data16[1] = htons(sequenceNumber);
	icmpHeader.icmp6_cksum = checksum(&icmpHeader, sizeof(icmphdr));

	request.sequenceNumber = sequenceNumber;
	request.tx = std::chrono::steady_clock::now();

	return ntp::sockets::socket_error != ntp::sockets::sendto(m_sockfd, (const char*)&icmpHeader, sizeof(icmpHeader), 0, (sockaddr*)&destAddr, destAddr_len);
}

bool Pinger::wait_For_ping_reply(uint16_t expectedSequenceNumber, PingRequest & pingRequest)
{
	pingRequest.err_msg.clear();
	pingRequest.receiving_from_wrong_source = pingRequest.receiving_failed = false;

	switch (m_af_net)
	{
	case AF_INET:
		return wait_For_ping_reply_ipv4(expectedSequenceNumber, pingRequest);
	case AF_INET6:
		return wait_For_ping_reply_ipv6(expectedSequenceNumber, pingRequest);
	default:
		assert(0);
		throw std::exception();
	}
}



bool Pinger::wait_For_ping_reply_ipv4(uint16_t expectedSequenceNumber, PingRequest &pingRequest)
{
	char recvBuffer[sizeof(iphdr) + sizeof(icmphdr)];
	sockaddr_in fromAddr{ 0 };
	int fromAddrLen = sizeof(fromAddr);
	const int bytesReceived = ntp::sockets::recvfrom(m_sockfd, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
	pingRequest.rx = std::chrono::steady_clock::now();

	if (ntp::sockets::socket_error == bytesReceived) {
		const auto err = ntp::sockets::SocketError::get_last_error();
		pingRequest.err_msg = "Pinger: recv response failed: " + err.msg;
		pingRequest.receiving_failed = true;
		return false;
	}

	if (0 != memcmp(pingRequest.target_ip, &fromAddr.sin_addr, sizeof(fromAddr.sin_addr))) {
		pingRequest.receiving_from_wrong_source = true;
		pingRequest.err_msg = "Unexpected replay sequence";
		return false;
	}

	if (bytesReceived < static_cast<int>(sizeof(iphdr) + sizeof(icmphdr))) {
		pingRequest.err_msg = "recv not enough data received";
		return false;
	}

	//on ipv4, the icmp respose include additional to the icmp header the ip.
	icmphdr *receivedIcmpHeader(nullptr);
	iphdr *ipHeader(nullptr);
	ipHeader = reinterpret_cast<iphdr *>(recvBuffer);
	receivedIcmpHeader = reinterpret_cast<icmphdr *>(recvBuffer + sizeof(*ipHeader));

	receivedIcmpHeader->un.echo.id = ntohs(receivedIcmpHeader->un.echo.id);
	receivedIcmpHeader->un.echo.sequence = ntohs(receivedIcmpHeader->un.echo.sequence);

	if (receivedIcmpHeader->un.echo.id != pingRequest.id || receivedIcmpHeader->un.echo.sequence != expectedSequenceNumber) {

		//If multiple pings take place in paralell we may 'lift' repsonse that are addressd to different pinger instance.
		pingRequest.receiving_from_wrong_source = receivedIcmpHeader->un.echo.id != pingRequest.id;
		pingRequest.err_msg = "Unexpected replay sequence";
		return false;
	}

	pingRequest.receivedTTL = ipHeader->ttl;
	pingRequest.receivedType = receivedIcmpHeader->type;
	pingRequest.receivedCode = receivedIcmpHeader->code;

	switch (receivedIcmpHeader->type)
	{
	case ipv4::icmp::echoreply:
		return true;
	case ipv4::icmp::dest_unreach:
		pingRequest.err_msg = getDestinationUnreachableMessage(pingRequest.receivedCode);
		return false;
	case ipv4::icmp::time_expire:
		if (0 == pingRequest.receivedCode)
			pingRequest.err_msg = "Time to Live Exceeded in Transit";
		else
			pingRequest.err_msg = "Fragment Reassembly Time Exceeded";
		return false;

	default:
		pingRequest.err_msg = "Unexpected replay";
		return false;
	}

}

bool Pinger::wait_For_ping_reply_ipv6(uint16_t expectedSequenceNumber, PingRequest &pingRequest)
{
	char recvBuffer[sizeof(ipv6hdr) + sizeof(icmp6_hdr)];
	sockaddr_in6 fromAddr{ 0 };
	int fromAddrLen = sizeof(fromAddr);
	const int bytesReceived = ntp::sockets::recvfrom(m_sockfd, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
	pingRequest.rx = std::chrono::steady_clock::now();

	if (ntp::sockets::socket_error == bytesReceived) {
		const auto err = ntp::sockets::SocketError::get_last_error();
		pingRequest.err_msg = "Pinger: recv response failed: " + err.msg;
		pingRequest.receiving_failed = true;
		return false;
	}

	if (0 != memcmp(pingRequest.target_ip, &fromAddr.sin6_addr, sizeof(fromAddr.sin6_addr))) {
		pingRequest.receiving_from_wrong_source = true;
		pingRequest.err_msg = "Unexpected replay sequence";
		return false;
	}

	if (bytesReceived < 8) {//on ipv6 the respose has minimum amount of data
		pingRequest.err_msg = "recv not enough data received";
		return false;
	}

	icmp6_hdr *receivedIcmpHeader(nullptr);
	//on ipv6, the icmp may not include additional to the icmp header the ipheader(so the ttl info may not be available).
	if (bytesReceived >= static_cast<int>(sizeof(ipv6hdr) + sizeof(icmp6_hdr)))
	{
		ipv6hdr *ipHeader(nullptr);
		ipHeader = reinterpret_cast<ipv6hdr*>(recvBuffer);
		receivedIcmpHeader = reinterpret_cast<icmp6_hdr *>(recvBuffer + sizeof(*ipHeader));
		pingRequest.receivedTTL = ipHeader->hop_limit;
	}
	else {
		receivedIcmpHeader = reinterpret_cast<icmp6_hdr *>(recvBuffer);
		pingRequest.receivedTTL = -1;//not available
	}

	receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[0] = ntohs(receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[0]);
	receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[1] = ntohs(receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[1]);
	const uint16_t echo_id = receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[0];
	const uint16_t echo_sequence = receivedIcmpHeader->icmp6_dataun.icmp6_un_data16[1];

	if (echo_id != pingRequest.id || echo_sequence != expectedSequenceNumber) {
		//If multiple pings take place in paralell we may 'lift' repsonse that are addressd to different pinger instance.
		pingRequest.receiving_from_wrong_source = echo_id != pingRequest.id;
		pingRequest.err_msg = "Unexpected replay sequence";
		return false;
	}

	pingRequest.receivedType = receivedIcmpHeader->icmp6_type;
	pingRequest.receivedCode = receivedIcmpHeader->icmp6_code;

	switch (receivedIcmpHeader->icmp6_type)
	{
	case ipv6::echoreply:
		return true;
	case ipv6::dest_unreach:
		pingRequest.err_msg = getDestinationUnreachableMessage(pingRequest.receivedCode);
		return false;
	case ipv6::time_expire:
		if (0 == pingRequest.receivedCode)
			pingRequest.err_msg = "Time to Live Exceeded in Transit";
		else
			pingRequest.err_msg = "Fragment Reassembly Time Exceeded";
		return false;

	default:
		pingRequest.err_msg = "Unexpected replay";
		return false;
	}

}
