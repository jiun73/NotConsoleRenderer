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

#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <thread>
#include <functional>

#include "ntp_client.h"
#include "utils.h"
#include "sockets_helper.h"

#define RX_DEFAULT_TIMEOUT_MILLISEC	  5000

using namespace ntp;

std::shared_ptr<ntp::IClient> ntp::IClient::create_instance() { return std::make_shared<ntp::Client>(); }

const uint16_t ntp::Client::STATS_CALC_WINDOW_SIZE(50);

ntp::Client::Client() :
	m_should_use_system_clock(false),
	m_socket(ntp::sockets::invalid_socket),
	m_p_si_addr_size(0),
	m_p_si_addr(nullptr),
	m_address_family(AF_UNSPEC)
{
	static_assert(ntp::Client::STATS_CALC_WINDOW_SIZE >= 2, "value of ntp::Client::STATS_CALC_WINDOW_SIZE at least 2.");

	ntp::init();
	ntp::utils::switch_os_accurate_clock(true);
	m_stats.clear();
	m_ntp_server_state.clear();
	m_asynch_query_thread = std::thread(std::bind(&ntp::Client::asynch_query_work, this));
}


ntp::Client::~Client()
{
	if (m_asynch_query_thread.joinable())
	{
		//push dummy job on the thread, signaling it to break;
		m_qj.push_wait_query(nullptr);
		m_asynch_query_thread.join();
	}

	cleanup_socket_and_address();

	ntp::utils::switch_os_accurate_clock(false);
}

void ntp::Client::init(const ClientSettings& settings)
{
	m_ntp_server_state.clear();
	m_stats.clear();

	m_settings = settings;
	if (std::chrono::nanoseconds(0) > m_settings.clock_adjustment_rules.min_clock_adjustment)
		m_settings.clock_adjustment_rules.min_clock_adjustment = std::chrono::nanoseconds(0);

	if (m_settings.clock_adjustment_rules.min_clock_adjustment > m_settings.clock_adjustment_rules.max_clock_adjustment)
		m_settings.clock_adjustment_rules.max_clock_adjustment = m_settings.clock_adjustment_rules.min_clock_adjustment;

	if (std::chrono::nanoseconds(0) >= m_settings.clock_adjustment_rules.max_latency)
		m_settings.clock_adjustment_rules.max_latency = std::chrono::seconds(60);//big enough to effectivly disable it.

	if (std::chrono::microseconds(0) > m_settings.fta.clock_adjustment_jitter_min)
		m_settings.fta.clock_adjustment_jitter_min = std::chrono::microseconds(0);

	m_should_use_system_clock = LocalTimeSource::SystemTime == m_settings.lts;
}


static std::string StatusToString(Status status)
{
	const int nb_states = 13;
	static const char status_s[nb_states][50] = {
		"Ok", "Unknown Error", "Init Sock Err", "Create Socket Err", "Tx Message Err",
		"Rx Msg Err", "Rx Msg Timeout", "Set OS Time Err", "Admin Rights Needed",
		"Max Time Adjustment Delta", "Min Time Adjustment Delta", "Max Latency limit", "Server Out Of Synch"
	};

	auto s = (unsigned int)status;
	return (s >= 0 && s <= (nb_states - 1)) ? status_s[s] : "Unexpected State";
}

static std::string TimeResultToString(const ntp::DateTime& sampleTime, const ntp::DateTime& nowTime, const Metrics& mtr)
{
	//delta_from_internal_clock_ns
	std::string net_staus_str("Net stats: N/A"), clock_adjustment_stats_str("Clock adjustment: N/A");
	const std::string timesample_str = ntp::utils::format_string("sample clock: reference => %s | now => %s ,Inner clock delta(ms): %.6f", sampleTime.as_local_time_string().c_str(), nowTime.as_local_time_string().c_str(), mtr.delta_from_internal_clock_ns / 1e6);
	const std::string offset_breakdown_str = ntp::utils::format_string("Query breakdown (ms): offset: %.6f delay: %.6f delta(drift correction): %.6f delta: %.6f drift_rate: %.6f", mtr.offset_ns / 1e6, mtr.delay_ns / 1e6, mtr.delta_no_drift_ns / 1e6, mtr.delta_with_drift_ns / 1e6, mtr.drift_rate);

	if (mtr.ds.has_stats())
	{
		net_staus_str = ntp::utils::format_string("Network stats (%u samples ms)         : jitter %.6f | avg delay %.6f", (unsigned int)mtr.ds.nb_samples, mtr.ds.network_latency_jitter_ns / 1e6, mtr.ds.network_latency_avg_ns / 1e6);
		clock_adjustment_stats_str = ntp::utils::format_string("Clock adjustment stats (%u samples ms): jitter %.6f | avg delta %.6f", (unsigned int)mtr.ds.nb_samples, mtr.ds.clock_adjustment_jitter_ns / 1e6, mtr.ds.clock_adjustment_avg_ns / 1e6);
	}
	return ntp::utils::format_string("%s\n%s\n%s\n%s\n", timesample_str.c_str(), offset_breakdown_str.c_str(), net_staus_str.c_str(), clock_adjustment_stats_str.c_str());
}

static double calculateMean(const std::list<double>& values) {
	if (values.empty()) return 0.0;
	const auto sum = std::accumulate(values.begin(), values.end(), 0.0);
	return sum / values.size();
}

static double calculateStandardDeviation(const std::list<double>& values, double* out_avg = 0)
{
	double sum = std::accumulate(values.begin(), values.end(), 0.0);;
	double sumSquared = 0.0;
	int numSlots = static_cast<int>(values.size());

	// Calculate the sum of values
	for (double val : values) {
		sum += val;				  //sum of values
		sumSquared += (val * val);//sum of squared value
	}

	// Calculate the average value
	const double averageVal = sum / numSlots;

	// Calculate the mean squared value
	const double meanSquaredAvg = sumSquared / numSlots;

	// Calculate the variance
	const double variance = meanSquaredAvg - (averageVal * averageVal);

	// Calculate the standard deviation.
	const double std_dev = std::sqrt(std::abs(variance));

	if (out_avg) *out_avg = averageVal;
	return std_dev;
}

static double calculateJitter(const std::list<double>& values, double* out_avg = 0) {
	//The standard deviation of deltas is actualy a jitter.
	if (2 > values.size()) {
		if (out_avg)
			*out_avg = values.empty() ? 0 : values.front();
		return 0;
	}

	std::list<double> values_delta;

	for (auto it = std::next(values.begin()); it != values.end(); ++it)
		values_delta.push_back(*it - *std::prev(it));

	return calculateStandardDeviation(values_delta, out_avg);
}

ResultPtr ntp::Client::query(ntp::EndPointPtr ntpServerEp, bool invalidate_clock)
{
	if (!m_ntp_server_state.m_ep)
		invalidate_clock = true;

	if (!ntpServerEp || ntpServerEp->empty())
	{
		if (!m_ntp_server_state.m_ep || m_ntp_server_state.m_ep->empty())
		{
			if (m_settings.error_callback)
				m_settings.error_callback(*this, "ntp::Client::query error: empty endpoint provided, while no active endpoint configured", m_settings.userParam);

			assert(0);
			throw std::exception();
		}

		ntpServerEp = m_ntp_server_state.m_ep;//remember the last one
	}

	ResultPtr result(new Result());
	result->m_server_ep = ntpServerEp;

	static const auto IsServerMsgTransmissionError = [](Status st) -> bool {
		return Status::ReceiveMsgErr == st || Status::ReceiveMsgTimeout == st || Status::SendMsgErr == st || Status::ServerOutOfSynch == st;
	};

	try
	{
		//If user choosed invalidate the inner clock or the hostname/ip was changed, clear/reset all statisics, history, clocks and time info
		//and start again
		if (invalidate_clock || *ntpServerEp != *m_ntp_server_state.m_ep)// if endpoint changed, reinit
		{
			if (Status::Ok != (result->m_status = init(ntpServerEp)))
				return result;//init socket failed.
		}

		++m_stats.m_nb_server_calls;
		++m_stats.m_nb_stats_server_calls;

		const auto min_stats_calculation_hist_size = STATS_CALC_WINDOW_SIZE / 2;

		auto update_m_queries_success_rate = [&]() {
			if (1 <= m_stats.m_nb_stats_server_calls)
				m_stats.m_queries_success_rate = static_cast<double>(m_stats.m_nb_stats_server_calls - m_stats.m_nb_server_msg_transmission_errors) / m_stats.m_nb_stats_server_calls;
		};

		if (!m_qj.push_wait_query(result))// NTP server query
		{
			if (IsServerMsgTransmissionError(result->m_status)) {
				++m_stats.m_nb_server_msg_transmission_errors;
				update_m_queries_success_rate();
			}
			return result;//time value was not set on the result.
		}

		update_m_queries_success_rate();

		if (m_stats.has_queries_success_rate() && STATS_CALC_WINDOW_SIZE < m_stats.m_nb_stats_server_calls)//time recalculate the [queries_success_rate] -> start collectingfrom fresh.
			m_stats.m_nb_stats_server_calls = m_stats.m_nb_server_msg_transmission_errors = 0;

		//By now we've got 'fresh' time data froms server:
		m_stats.m_prev_sucessful_query_time = result->m_extraction_time;

		//set the result's metrix info
		result->m_mtr.delay_ns = m_ntp_server_state.m_delay_ns;//latency
		result->m_mtr.offset_ns = m_ntp_server_state.m_offset_ns;//offset determined by the NTP server
		result->m_mtr.delta_with_drift_ns = m_ntp_server_state.m_delta_with_drift_ns;//clock delta
		result->m_mtr.delta_no_drift_ns = m_ntp_server_state.m_delta_no_drift_ns;//clock delta after drift correction.

		result->m_mtr.drift_rate = m_ntp_server_state.m_drifts.m_drift_rate;//rate, in which the internal clock is drifting over time.
		const auto warmup_calls = 4;//Do not include in the stats the 1st X calls
		if (m_stats.m_nb_server_calls > warmup_calls)
		{
			//calc clock delta statistics:
			m_stats.m_clock_deltas.push_back(m_settings.mitigate_clock_drift ? result->m_mtr.delta_no_drift_ns : result->m_mtr.delta_with_drift_ns);
			if (m_stats.m_clock_deltas.size() > STATS_CALC_WINDOW_SIZE)
				m_stats.m_clock_deltas.erase(m_stats.m_clock_deltas.begin());

			if (min_stats_calculation_hist_size <= m_stats.m_clock_deltas.size())
				result->m_mtr.ds.clock_adjustment_jitter_ns = calculateJitter(m_stats.m_clock_deltas, &result->m_mtr.ds.clock_adjustment_avg_ns);

			//calc network latency statistics:
			m_stats.m_network_latency.push_back(result->m_mtr.delay_ns);
			if (m_stats.m_network_latency.size() > STATS_CALC_WINDOW_SIZE)
				m_stats.m_network_latency.erase(m_stats.m_network_latency.begin());

			if (min_stats_calculation_hist_size <= m_stats.m_network_latency.size())
				result->m_mtr.ds.network_latency_jitter_ns = calculateJitter(m_stats.m_network_latency, &result->m_mtr.ds.network_latency_avg_ns);

			result->m_mtr.ds.nb_samples = static_cast<uint16_t>(m_stats.m_clock_deltas.size());
			/*
			Should be the same...
			result->m_mtr.ds.nb_samples = static_cast<uint16_t>(m_stats.m_network_latency.size());
			*/

		}//if (m_stats.nb_server_calls > warmup_calls)

		m_ntp_server_state.m_sd = result->m_mtr.ds;

		const bool last_set_clock_had_jitter_info = m_ntp_server_state.m_clock_update_result && m_ntp_server_state.m_clock_update_result->m_mtr.ds.has_stats();
		const bool has_jitter_info = result->m_mtr.ds.has_stats();
		const bool time_data_is_set = has_time_private();
		const bool fine_tunning_over = is_fine_tunning_over();
		const auto should_fine_tune_clock = [&]() ->bool
		{
			if (fine_tunning_over) return false;
			return
				(
					//no jitter info collected on the prev update result that was set, but the current one has it,
					//so update the time (will tale place only one the 1st samples
				(!last_set_clock_had_jitter_info && has_jitter_info)
					||
					(
						//The adjustment is below the minimum but the jitter of the new result is more promissing then the prev update result.
						//ntp::Status::TimeAdjustLimitMin == result->m_status &&
						has_jitter_info && last_set_clock_had_jitter_info &&
						m_ntp_server_state.m_clock_update_result->m_mtr.ds.network_latency_jitter_ns >= result->m_mtr.ds.network_latency_jitter_ns
						)
					);
		};

		const auto should_synch_post_fine_tune = [&]() ->bool {
			if (!fine_tunning_over) return false;
			return ntp::Status::TimeAdjustLimitMin != result->m_status ||
				(
					m_ntp_server_state.m_sd.has_stats() && m_settings.minimal_clock_adjustment_rules.max_network_jitter >=
					std::chrono::duration_cast<std::chrono::microseconds>(ntp::nanoseconds_fp(m_ntp_server_state.m_sd.network_latency_jitter_ns))
					) ||
					(
						m_ntp_server_state.m_sd.has_stats() && m_settings.minimal_clock_adjustment_rules.max_clock_adjustment_jitter >=
						std::chrono::duration_cast<std::chrono::microseconds>(ntp::nanoseconds_fp(m_ntp_server_state.m_sd.clock_adjustment_jitter_ns))
						);
		};

		if (
			//In any case the inner clock usage we need to be updated at least once.
			//otherwise we will always be "too far" from the NTP server.
			!time_data_is_set
			|| should_synch_post_fine_tune()
			|| should_fine_tune_clock()
			)
		{
			if (std::chrono::steady_clock::time_point::min() == m_ntp_server_state.m_fine_tune_procedure_start_time)
				m_ntp_server_state.m_fine_tune_procedure_start_time = result->m_extraction_time;

			if (m_should_use_system_clock)
			{
				switch (ntp::utils::set_system_clock(result->now().time()))
				{
				case ntp::utils::SetSystemClockResult::Failed:
					result->m_status = Status::SetOsTimeErr;
					break;
				case ntp::utils::SetSystemClockResult::Failed_AdminRightsNeeded:
					result->m_status = Status::AdminRightsNeeded;
					break;
				}
			}
			result->m_used_for_setting_clock = true;
		}

		if (result->m_used_for_setting_clock) {
			//update clock if all terms are met
			std::unique_lock<std::shared_mutex> clock_lock(m_clock_lock);
			m_ntp_server_state.m_time = result->m_time;
			m_ntp_server_state.m_prev_clock_update_time = result->m_extraction_time;
			m_ntp_server_state.m_clock_update_result = result;
		}

		if (has_time_private())
			result->m_mtr.delta_from_internal_clock_ns = std::chrono::abs(std::chrono::duration_cast<ntp::nanoseconds_fp>(std::chrono::nanoseconds(result->now().time() - clock()->now().time()))).count();

		{
			std::unique_lock<std::shared_mutex> mtr_clock(m_mtr_lock);
			m_ntp_server_state.m_last_result_mtr.reset(new ntp::Metrics(result->mtr()));
		}

		if (result->m_used_for_setting_clock &&  m_settings.clockset_callback)
			m_settings.clockset_callback(*this, clock(), m_settings.userParam);

		manage_drifts_table(result);

		return result;
	}
	catch (const std::exception& exc)
	{
		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client exception cought on query : " << exc.what();
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}

		std::cerr << exc.what();
		result->m_status = Status::UnknownErr;
		return result;
	}
}

ntp::ManagedClockPtr  ntp::Client::clock() const {
	std::shared_lock<std::shared_mutex> clock_lock(m_clock_lock);
	if (!has_time_private()) return nullptr;
	return ManagedClockPtr(new ManagedClock(m_ntp_server_state.m_time, m_ntp_server_state.m_prev_clock_update_time, m_settings.mitigate_clock_drift ? m_ntp_server_state.m_drifts.m_drift_rate : 0.0));
}

const ServerInfoPtr ntp::Client::server_info() const {
	auto res = last_clock_update_query_result();
	return res ? last_clock_update_query_result()->server_info() : nullptr;
}

ntp::MetricsPtr ntp::Client::mtr() const
{
	ntp::MetricsPtr ptr;
	{
		std::shared_lock<std::shared_mutex> mtr_lock(m_mtr_lock);
		ptr = m_ntp_server_state.m_last_result_mtr;
	}
	return ptr;
}

ntp::FineTuneOverReason ntp::Client::finetune_over_reason() const { return m_stats.m_finetune_over; }

/*  Private Methods  */

ntp::Status ntp::Client::init(ntp::EndPointPtr ntpServerEp)
{
	//since we are resetting the targeted ntp server, we will reset the statistics the time related values stored.
	m_stats.clear();
	{
		std::unique_lock<std::shared_mutex> clock_lock(m_clock_lock);
		m_ntp_server_state.clear();
	}

	m_ntp_server_state.m_server_com_init_start_time = std::chrono::steady_clock::now();

	if (!ntp::sockets::is_socket_lib_initialized()) {
		if (m_settings.error_callback)
			m_settings.error_callback(*this, "ntp::Client::init failed on sockets init", m_settings.userParam);

		return Status::InitSockErr;
	}

	//******************************************
	//	Create address + socket and bind them
	//******************************************

	cleanup_socket_and_address();

	//attempt to classify/resolve the host name:
	ntp::sockets::host_name_resolving_filter ip_filter = ntp::sockets::host_name_resolving_filter::any;
	switch (m_settings.version)
	{
	case ClientSettings::RequestedProtocolVersion::AutoSelect://prefer compatability
	case ClientSettings::RequestedProtocolVersion::V4:
		ip_filter = ntp::sockets::host_name_resolving_filter::prefer_ip_v4;
		break;
	case ClientSettings::RequestedProtocolVersion::V3://only support IPv4
		ip_filter = ntp::sockets::host_name_resolving_filter::ip_v4_only;
		break;

	default:
		assert(0);
		throw std::exception();
	}
	ntp::sockets::addr_type addr_type;
	const auto ip_string = ntp::sockets::host_name_to_ip(ntpServerEp->host, addr_type, ip_filter);
	if (ip_string.empty())
	{
		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::init failed resolving the server's hostname/IP Address.";
			if (ClientSettings::RequestedProtocolVersion::V3 == m_settings.version)
				oss << std::endl << "The client was configured to work with NTP v3, try switching to AutoSelect or NTP v4.";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}
		return Status::InitSockErr;
	}

	ntp::ProtocolVersion client_operating_protocol_ver = ntp::ProtocolVersion::V3;

	switch (addr_type)
	{
	case ntp::sockets::addr_type::ipv4:
	{
		switch (m_settings.version)
		{
		case ClientSettings::RequestedProtocolVersion::AutoSelect:
		case ClientSettings::RequestedProtocolVersion::V3:
			client_operating_protocol_ver = ntp::ProtocolVersion::V3;
			break;
		case ClientSettings::RequestedProtocolVersion::V4:
			client_operating_protocol_ver = ntp::ProtocolVersion::V4;
			break;
		default:
			assert(0);
			throw std::exception();
		}

		m_address_family = AF_INET;
		m_p_si_addr_size = sizeof(sockaddr_in);
		m_p_si_addr = malloc(m_p_si_addr_size);
		memset(m_p_si_addr, 0, m_p_si_addr_size);
		auto addr = (sockaddr_in*)m_p_si_addr;
		addr->sin_family = m_address_family;
		addr->sin_port = htons(ntpServerEp->port);

		if (1 != inet_pton(m_address_family, ip_string.c_str(), &addr->sin_addr))
		{
			const std::string errStr = ntp::sockets::SocketError::get_last_error().msg;
			cleanup_socket_and_address();
			if (m_settings.error_callback) {
				std::stringstream oss; oss << "ntp::Client::init failed resolving a server host/address: " << errStr;
				m_settings.error_callback(*this, oss.str(), m_settings.userParam);
			}
			return Status::CreateSocketErr;
		}

		break;
	}
	case ntp::sockets::addr_type::ipv6:
	{
		//Only ntp v4 supports IP v6 (officialy)
		switch (m_settings.version)
		{
		case ClientSettings::RequestedProtocolVersion::AutoSelect:
		case ClientSettings::RequestedProtocolVersion::V4:
			client_operating_protocol_ver = ntp::ProtocolVersion::V4;
			break;
		case ClientSettings::RequestedProtocolVersion::V3:
			//only ntp v4 supports IP v6 (officialy)
			cleanup_socket_and_address();
			if (m_settings.error_callback) {
				std::stringstream oss; oss << "ntp::Client::init user specificly requested NTP v3, but server endpoint resolved to IP v6.";
				m_settings.error_callback(*this, oss.str(), m_settings.userParam);
			}
			return Status::InitSockErr;
			break;
		default:
			assert(0);
			throw std::exception();
		}

		m_address_family = AF_INET6;
		m_p_si_addr_size = sizeof(sockaddr_in6);
		m_p_si_addr = malloc(m_p_si_addr_size);
		memset(m_p_si_addr, 0, m_p_si_addr_size);
		auto addr = (sockaddr_in6*)m_p_si_addr;
		addr->sin6_family = m_address_family;
		addr->sin6_port = htons(ntpServerEp->port);
		if (1 != inet_pton(m_address_family, ip_string.c_str(), &addr->sin6_addr))
		{
			const std::string errStr = ntp::sockets::SocketError::get_last_error().msg;
			cleanup_socket_and_address();
			if (m_settings.error_callback) {
				std::stringstream oss; oss << "ntp::Client::init failed resolving a server host/address: " << errStr;
				m_settings.error_callback(*this, oss.str(), m_settings.userParam);
			}
			return Status::CreateSocketErr;
		}
		break;
	}
	default:
		assert(0);
		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::init unsupported address family";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}
		return Status::InitSockErr;
	}

	if ((m_socket = socket(m_address_family, SOCK_DGRAM, IPPROTO_UDP)) == ntp::sockets::invalid_socket) {
		const std::string errStr = ntp::sockets::SocketError::get_last_error().msg;
		cleanup_socket_and_address();
		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::init failed creating a socket: " << errStr;
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}

		return Status::CreateSocketErr;
	}

	const int rx_timeout = (0 >= m_settings.clock_adjustment_rules.server_response_timeout.count()) ? RX_DEFAULT_TIMEOUT_MILLISEC : static_cast<int>(m_settings.clock_adjustment_rules.server_response_timeout.count());

	if (0 != ntp::sockets::set_socket_recv_timeout(m_socket, std::chrono::milliseconds(rx_timeout)))	{
		cleanup_socket_and_address();
		return Status::CreateSocketErr;
	}

	m_ntp_server_state.m_protocol_ver = client_operating_protocol_ver;
	m_ntp_server_state.m_ep = ntpServerEp;
	m_ntp_server_state.m_ep_ip = std::make_shared<IPAddress>();
	if (AF_INET == m_address_family) {
		m_ntp_server_state.m_ep_ip->ver = IPAddress::Version::V4;
		memcpy(&m_ntp_server_state.m_ep_ip->addr.v4, &((sockaddr_in*)m_p_si_addr)->sin_addr, sizeof(m_ntp_server_state.m_ep_ip->addr.v4));
	}
	else {
		m_ntp_server_state.m_ep_ip->ver = IPAddress::Version::V6;
		memcpy(&m_ntp_server_state.m_ep_ip->addr.v6, &((sockaddr_in6*)m_p_si_addr)->sin6_addr, sizeof(m_ntp_server_state.m_ep_ip->addr.v6));
	}

	return Status::Ok;
}

void ntp::Client::cleanup_socket_and_address()
{
	if (m_p_si_addr) {//release the current address mem
		free(m_p_si_addr);
		m_p_si_addr = nullptr;
		m_p_si_addr_size = 0;
		m_address_family = AF_UNSPEC;
	}

	if (ntp::sockets::invalid_socket != m_socket) {//release the current socket
		ntp::sockets::shutdown_socket(m_socket, ntp::sockets::shutdown_what::both);
		ntp::sockets::close_socket(m_socket);
		m_socket = ntp::sockets::invalid_socket;
	}
}

/*
This method performs the server's query call and prforms time mesurements:
Here's how you can calculate the round-trip delay using the timestamps received from the NTP server:
1. Send Timestamp		(T1):  The client records the local time (in timestamp format) when it sends the NTP request packet to the server.
2. Receive Timestamp	(T2):  When the NTP server receives the request, it records its local time (in timestamp format).
3. Transmit Timestamp	(T3):  The NTP server includes its local time (in timestamp format) in the response packet when it sends the response back to the client.
4. Destination Timestamp (T4): The client records the local time (in timestamp format) when it receives the NTP response packet from the server.

Round-Trip Delay:
	The round-trip delay represents the time it takes for a request packet to travel from the client to the server and
	for the corresponding response packet to travel back from the server to the client.
	It includes both the network propagation delay and any processing delays.
Round-Trip-Delay = ( (T4 - T1) - (T3 - T2) )/2

Network-Propagation-Delay:
In order to account for the fact that the delay is the round-trip time, the overall difference needs to be split between the outbound and inbound paths:
Network-Propagation-Delay= (Round-Trip-Delay)/2


Local-Clocks-Offset:
	The local clock offset represents the difference between the client's local clock and the server's clock.
	This offset is used to adjust the client's clock to synchronize it with the server's clock.

Local-Clocks-Offset = ( (T2 - T1) + (T3 - T4) )/2.


When adding the offset to the client's local clock, we should take the round-trip delay into account to ensure the most accurate time synchronization.
To adjust the client's local clock with the calculated offset,
we should add the round-trip delay (in other words the RTD) to the calculated offset.
This compensates for the time it takes for the request and response to travel each way:

Adjusted-Offset = Local-Clocks-Offset + RTD
By adding the RTD to the calculated offset,
we ensure that the adjustment is made based on the time it took for the request and response to travel both ways between the client and the server.

// Update local time:
adjustedTime = receiveTime.AddSeconds(adjustedOffset);

// Set the local machine time
SetLocalMachineTime(adjustedTime);

*/

bool ntp::Client::query_private(ResultPtr in_out_result)
{
	memset(&m_out_ntp_msg, 0, sizeof(m_out_ntp_msg));

	//11100011 (client mode + NTPv4)
	//leap indicator should be set only by server (server is not consuming it from client)
	m_out_ntp_msg.pkt.li_vn_mode = create_li_vn_mode(ntp::AssociationModes::Client, LeapIndicator::NoWarning, m_ntp_server_state.m_protocol_ver);
	//Stratum level of the local clock
	m_out_ntp_msg.pkt.stratum = static_cast<uint8_t>(StratumLevel::Unspecified);
	//Maximum interval between successive messages. The value is evalueted value^2 in seconds
	m_out_ntp_msg.pkt.poll = calculate_clients_polling_min_intervals(1);//requesting one second
	//Precision of the local clock
	m_out_ntp_msg.pkt.precision = calculate_precision(std::chrono::microseconds(1));
	//Total round trip delay time
	m_out_ntp_msg.pkt.rootDelay.all = 256;
	//Max error allowd from primary clock source
	m_out_ntp_msg.pkt.rootDispersion.all = 256;

	//UDP packets have the possiblity to be eather duplicated (arrive twice) or be a bogus packet.
	//-to avoid bogus packets we are placing on the packet unique key on the packet (on the output tx, that the server should return)
	// when the server will return a replay we will check that the key matchs.
	//-to avoid duplicated packets we will store the value the server placed on the tx on his respose,
	// and make sure that the next respose will have different value on it.

	//placing 'unique' special value (non valid time) on the tx fields of outgoing pakcket.
	//This will help verify that the response will come from the server we're sending the packet to, and that the response is associated with this query.
	const int64_t request_unique_key = std::chrono::steady_clock::now().time_since_epoch().count() / 2;
	memcpy(&m_out_ntp_msg.pkt.txTm.all, &request_unique_key, sizeof(request_unique_key));

	//send the message
	const bool use_system_clock = (m_should_use_system_clock || !has_time_private());

	std::chrono::nanoseconds drift_component(0);
	const time_point_t t1 = use_system_clock ?
		//if utilizing local clock as reference (or at least as starting point), otherwise relay on steady_clock.
		std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()) : now_private(false, drift_component);

	const auto high_now = std::chrono::steady_clock::now();
	const auto t1_b = high_now;

	if (ntp::sockets::socket_error == ntp::sockets::sendto(m_socket, m_out_ntp_msg.as_buffer, NTP_PACKET_LEN, 0, (sockaddr*)m_p_si_addr, m_p_si_addr_size))
	{
		const std::string errStr = ntp::sockets::SocketError::get_last_error().msg;
		if (m_settings.warnning_callback) {
			std::stringstream oss; oss << "ntp::Client::Query: server request failed: " << errStr;
			m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);
		}
		in_out_result->m_status = Status::SendMsgErr;
		return false;
	}

	// receive message
	const int recv_bytes = ntp::sockets::recvfrom(m_socket, m_in_ntp_msg.as_buffer, NTP_PACKET_LEN, 0, (sockaddr*)m_p_si_addr, &m_p_si_addr_size);
	if (recv_bytes != NTP_PACKET_LEN)
	{
		std::stringstream oss;
		if (ntp::sockets::socket_error == recv_bytes)
		{
			const auto last_error = ntp::sockets::SocketError::get_last_error();
			oss << "ntp::Client::Query: recv response failed: " << last_error.msg;
			if(last_error.is_one_of({EWOULDBLOCK, EAGAIN, ETIMEDOUT}))
				in_out_result->m_status = Status::ReceiveMsgTimeout;
			else
				in_out_result->m_status = Status::ReceiveMsgErr;
		}
		else
		{
			oss << "ntp::Client::Query: recv response : " << "unexpected respose packet size.";
			in_out_result->m_status = Status::ReceiveMsgErr;
		}

		if (m_settings.warnning_callback)
			m_settings.warnning_callback(*this, oss.str(), m_settings.userParam);

		return false;
	}

	const time_point_t t4 = use_system_clock ? //nanoseconds from 1970
		std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()) :
		m_ntp_server_state.m_time + std::chrono::nanoseconds(std::chrono::steady_clock::now() - m_ntp_server_state.m_prev_clock_update_time);

	const auto t4_b = std::chrono::steady_clock::now();

	const auto leap_indicator = get_ntp_leap(m_in_ntp_msg.pkt.li_vn_mode);
	if (ntp::LeapIndicator::NotSynched == leap_indicator)
	{
		//Server reporting an error (it's not synched)
		in_out_result->m_status = Status::ServerOutOfSynch;

		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::Query: recv response, server reported it's out of sync, discarding respose";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}

		return false;
	}

	if (0 != memcmp(&m_in_ntp_msg.pkt.origTm, &request_unique_key, sizeof(request_unique_key))) {
		//the sender did not send back the value we've sent over txTm_s/f, it probebly did not came from our server,
		//or its a UDP bogus packets, another option is that it's duplicated respose belongs do a different query

		in_out_result->m_status = Status::ReceiveMsgErr;

		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::Query: recv response, server replayed with wrong unique id, discarding respose";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}

		return false;
	}
	
	const auto assoc_mode = get_ntp_mode(m_in_ntp_msg.pkt.li_vn_mode);
	const auto server_ntp_version = get_ntp_version(m_in_ntp_msg.pkt.li_vn_mode);
	const ntp::StratumLevel stratum(static_cast<ntp::StratumLevel>(m_in_ntp_msg.pkt.stratum));

	if (m_ntp_server_state.m_protocol_ver != server_ntp_version)
	{
		//Some server will conform to requested protocols. i.e will return ntpV4 respose even when supporting IPV6 like time.google.com
		//whose will normally be stratum 1 servers.
		//Others like microsoft's time.windows.com (stratum 3, not supporting ipv6) return only ntpV3 packets.
		//pool.ntp.org (stratum 3, not supporting ipv6), will return ntpV4 respose, and so will ntp21.vniiftri.ru (stratum 2).

		in_out_result->m_status = Status::ReceiveMsgErr;//The other end is not working in the requested version.

		if (m_settings.error_callback) {
			std::stringstream oss; oss << "ntp::Client::Query: recv response, server replayed with wrong protocol version, discarding respose";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}
		return false;
	}

	if (AssociationModes::ControlMessage == assoc_mode)
	{
		assert(0);//TODO: improve the treatment, add new Status code etc.., to the server side add also control messages
		in_out_result->m_status = Status::ReceiveMsgErr;

		//Server flaged the respose as "control message", look for the The Kiss of Death (KoD) code in the refid field
		if (0 == memcmp(m_in_ntp_msg.pkt.refId.ascii, KoD::AccessDenied, sizeof(m_in_ntp_msg.pkt.refId)) ||
			0 == memcmp(m_in_ntp_msg.pkt.refId.ascii, KoD::RateExceeded, sizeof(m_in_ntp_msg.pkt.refId)) ||
			0 == memcmp(m_in_ntp_msg.pkt.refId.ascii, KoD::AccessDenied_ResponseRequired, sizeof(m_in_ntp_msg.pkt.refId))
			)
		{
			assert(0);//TODO: improve the treatment, add new Status code etc.., to the server side add also control messages
		}

		if (m_settings.warnning_callback)
		{
			auto refid_str = ntp::parse_reference_identifier(m_in_ntp_msg.pkt.refId, stratum, server_ntp_version, assoc_mode);
			std::stringstream oss; oss << "ntp::Client::Query: recv response, server replayed control message: " << refid_str << " , discarding respose.";
			m_settings.error_callback(*this, oss.str(), m_settings.userParam);
		}

		return false;
	}
	else if (AssociationModes::Server != assoc_mode) {
		in_out_result->m_status = Status::ReceiveMsgErr;//The other end is not configured as a server.
		return false;
	}

	std::chrono::nanoseconds ns;
	//convert server's response (reference time) from ntp time format to posix format, and store it.
	//this value is the last time that the clock on the remote server was "corrected/updated" by it's own timesource.
	ntp_time_to_posix(ns, ntohl(m_in_ntp_msg.pkt.refTm.segments.s), ntohl(m_in_ntp_msg.pkt.refTm.segments.f));
	const ntp::DateTime server_ref((ntp::time_point_t(ns)));

	//convert server's response (tx and rx) from ntp time format to posix format, and store it on t2_ns , t3_ns respectfully.
	ntp_time_to_posix(ns, ntohl(m_in_ntp_msg.pkt.rxTm.segments.s), ntohl(m_in_ntp_msg.pkt.rxTm.segments.f));
	const double t2_ns = ntp::nanoseconds_fp(ns).count();

	ntp_time_to_posix(ns, ntohl(m_in_ntp_msg.pkt.txTm.segments.s), ntohl(m_in_ntp_msg.pkt.txTm.segments.f));
	const double t3_ns = ntp::nanoseconds_fp(ns).count();

	const double t1_ns = ntp::nanoseconds_fp(t1.time_since_epoch()).count();//nanoseconds from 1970
	const double t4_ns = ntp::nanoseconds_fp(t4.time_since_epoch()).count();//nanoseconds from 1970
	const double t1_b_ns = ntp::nanoseconds_fp(t1_b.time_since_epoch()).count(); // nanoseconds from start measuring
	const double t4_b_ns = ntp::nanoseconds_fp(t4_b.time_since_epoch()).count();

	//network latency(round trip delay) RTD.
	const double network_roundtrip_delay_ns = (t4_b_ns - t1_b_ns) - (t3_ns - t2_ns);

	//Claculating the client clock offset from the server this, take into account the network latency:
	//The offset is calculated by subtracting the time it took for the request packet to reach the server(T2 - T1)
	//from the time it took for the response packet to reach the client(T3 - T4).This difference in times is due to the network latency.
	m_ntp_server_state.m_offset_ns = ((t2_ns - t1_ns) + (t3_ns - t4_ns)) / 2.0;//clocks offset.


	//The network-delay should be calculated in order to icluse it when adjusting the client's local clock:
	m_ntp_server_state.m_delay_ns = network_roundtrip_delay_ns / 2.0;//network latency

	//this is the "adjusted" offset.
	m_ntp_server_state.m_delta_with_drift_ns = m_ntp_server_state.m_offset_ns + network_roundtrip_delay_ns;
	m_ntp_server_state.m_drift_ns = static_cast<double>(drift_component.count());
	m_ntp_server_state.m_delta_no_drift_ns = m_ntp_server_state.m_delta_with_drift_ns - m_ntp_server_state.m_drift_ns;
	const std::chrono::nanoseconds delta_ns(static_cast<int64_t>(m_settings.mitigate_clock_drift ? m_ntp_server_state.m_delta_no_drift_ns : m_ntp_server_state.m_delta_with_drift_ns));
	const std::chrono::nanoseconds delta_ns_abs(std::chrono::abs(delta_ns));
	const std::chrono::nanoseconds server_precision_ns = ntp::parse_precision(m_in_ntp_msg.pkt.precision);
	auto sei = std::make_shared<ServerInfo>();
	sei->server_ep = m_ntp_server_state.m_ep;
	sei->server_ip = m_ntp_server_state.m_ep_ip;
	sei->server_reference_time = server_ref;
	sei->leap_indicator = leap_indicator;
	sei->stratum.value = stratum;
	sei->version = server_ntp_version;
	//no need to call ntohl on ntp::parse_reference_identifier() input (see comments)
	sei->ref_id = ntp::parse_reference_identifier(m_in_ntp_msg.pkt.refId, stratum, sei->version, assoc_mode);
	sei->min_query_intervals = std::chrono::seconds(static_cast<int>(std::pow(2, static_cast<int>(m_in_ntp_msg.pkt.poll))));

	ntp::get_chrono_duration_from_ntp_duration<uint16_t>(sei->root_delay, ntohs(m_in_ntp_msg.pkt.rootDelay.segments.s), ntohs(m_in_ntp_msg.pkt.rootDelay.segments.f));
	ntp::get_chrono_duration_from_ntp_duration<uint16_t>(sei->root_dispersion, ntohs(m_in_ntp_msg.pkt.rootDispersion.segments.s), ntohs(m_in_ntp_msg.pkt.rootDispersion.segments.f));

	if (m_ntp_server_state.m_sd.has_stats()) {
		sei->average_network_latency_abs = std::chrono::nanoseconds(static_cast<int64_t>(0.5 + std::abs(m_ntp_server_state.m_sd.network_latency_avg_ns)));
		sei->average_delta_abs = std::chrono::nanoseconds(static_cast<int64_t>(0.5 + m_ntp_server_state.m_sd.clock_adjustment_avg_ns));
	}
	else {
		sei->average_network_latency_abs = std::chrono::nanoseconds(static_cast<int64_t>(0.5 + std::abs(m_ntp_server_state.m_delay_ns)));
		sei->average_delta_abs = delta_ns_abs;
	}

	in_out_result->m_server_ext_info = sei;

	const bool allow_huge_clock_delta = !has_time_private() && m_settings.clock_adjustment_rules.ignore_max_clock_adjustment_on_start;

	if (delta_ns_abs > m_settings.clock_adjustment_rules.max_clock_adjustment && !allow_huge_clock_delta) {
		in_out_result->m_status = Status::TimeAdjustLimitMax;
		return false;
	}

	if (delta_ns_abs < m_settings.clock_adjustment_rules.min_clock_adjustment)//If the time adjustment is below the minimal threshold
		in_out_result->m_status = Status::TimeAdjustLimitMin;
	else if (m_ntp_server_state.m_delay_ns > ntp::nanoseconds_fp(m_settings.clock_adjustment_rules.max_latency).count())
		in_out_result->m_status = Status::LatencyLimitMax;
	else
		in_out_result->m_status = Status::Ok;

	double delta_weight = 1.0;

	//TODO: take into account that on the last minute of the month with leap indicator on,
	//		the clock/time should be smoothly transition into the end of the day (23:59:00) with one second compensation.
	//		that compensation should be made from 23:59:00 till 24:00:00.

	if (m_settings.smooth_clock_adjustmet)
	{
		//Set the slewing and stepping factors:
		//-Slewing factor: a constant that determines how quickly the client's clock is adjusted. The slewing factor is typically a small number, such as 0.01 or 0.001.
		const double slewing_factor = 0.01;//
		const double stepping_factor = 0.2;

		if (!allow_huge_clock_delta)
			delta_weight = delta_ns_abs > std::chrono::milliseconds(15) ? slewing_factor : stepping_factor;
	}

	//setting the new/adjusted time:
	in_out_result->m_time = t4 + std::chrono::nanoseconds(static_cast<int64_t>(0.5 + delta_ns.count()*delta_weight));
	//setting steady clock time reference to the new time.
	in_out_result->m_extraction_time = t4_b;

	return true;
}

void ntp::Client::asynch_query_work()
{
#ifdef WIN32
	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
	{/*printf("Failed to enter background mode (%d)\n", GetLastError());*/
	}
	else {/*printf("Current priority class is 0x%x\n", GetPriorityClass(GetCurrentProcess()));*/ }

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	//SetThreadAffinityMask(GetCurrentThread(), 1);// Lock thread to CPU core 0
#else //unix/linux
	//Set thread priority(Thread Scheduling Policy : SCHED_FIFO)
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);

	// Set thread affinity
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	//pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);// Lock thread to CPU core 0
#endif//WIN32

	while (true)
	{
		m_qj.query_thread_job_started_evt.wait();
		m_qj.query_thread_job_started_evt.reset();
		if (!m_qj.in_out_query_result) {
			m_qj.query_sucess = false;//signal to break;
			m_qj.query_thread_job_over_evt.set();
			break;
		}

		m_qj.query_sucess = query_private(m_qj.in_out_query_result);
		m_qj.in_out_query_result.reset();
		m_qj.query_thread_job_over_evt.set();
	}
}

void ntp::Client::manage_drifts_table(const ResultPtr query_result)
{
	if (!query_result->has_time() || !m_ntp_server_state.m_fine_tune_procedure_over)
		return;

	//By now should be "finetuned/synched".
	static const auto get_drift_file_path = [](std::string drift_file_name) ->std::string
	{
		if (std::filesystem::is_regular_file(drift_file_name))
			return drift_file_name;//its a path to an existing file.

		if (std::string::npos != drift_file_name.find('/') || std::string::npos != drift_file_name.find('\\'))
			return drift_file_name;//its a path to non exiting file.

		return (std::filesystem::current_path() / drift_file_name).string();
	};

	const static auto load_drift_table = [](std::list<std::pair<ntp::time_point_t, double>>& tbl, std::string file_path) ->void
	{
		// Load drift table from text file
		tbl.clear(); std::string line;
		std::ifstream file(file_path);
		if (!file) return;
		while (std::getline(file, line))
		{
			std::pair<ntp::time_point_t, double> entry;
			std::istringstream iss(line);
			std::string field;

			while (std::getline(iss, field, ';')) {
				size_t pos = field.find('=');
				if (pos != std::string::npos) {
					std::string key = field.substr(0, pos);
					std::string value = field.substr(pos + 1);
					if (key == "client_time_ns")// Convert posix time nanoseconds to system_clock::time_point
						entry.first = time_point_t(std::chrono::nanoseconds(std::stoll(value)));
					else if (key == "delta_ns")
						entry.second = std::stod(value);
				}
			}
			// Store entry in drift table
			tbl.push_back(entry);
		}//end of while (std::getline(file, line))
	};

	//[drift_table_backup_file]	points to the drift table backed up file and be loaded at startup.
	const std::string drift_table_backup_file = !m_settings.drift_table_backup_file_name.empty() ?
		get_drift_file_path(m_settings.drift_table_backup_file_name) : std::string();

	if (!drift_table_backup_file.empty())
	{
		if (m_ntp_server_state.m_drifts.m_drift_table.empty() && std::filesystem::is_regular_file(drift_table_backup_file))
			load_drift_table(m_ntp_server_state.m_drifts.m_drift_table, drift_table_backup_file);
	}

	m_ntp_server_state.m_drifts.m_drift_table.push_back(std::make_pair(query_result->now().time(), query_result->mtr().delta_with_drift_ns));

	while (500 < m_ntp_server_state.m_drifts.m_drift_table.size())
		m_ntp_server_state.m_drifts.m_drift_table.erase(m_ntp_server_state.m_drifts.m_drift_table.begin());

	if (50 > m_ntp_server_state.m_drifts.m_drift_table.size())
		return;

	if (!drift_table_backup_file.empty() && std::chrono::minutes(5) <= m_ntp_server_state.m_drifts.time_from_last_backup())
	{
		//writing backup drift file.
		std::ofstream drift_file(drift_table_backup_file, std::ios::trunc);
		for (const auto& drift_record : m_ntp_server_state.m_drifts.m_drift_table) {
			drift_file << "client_time_ns="
				<<
				std::chrono::nanoseconds(drift_record.first.time_since_epoch()).count()
				<< ";delta_ns=" << drift_record.second << std::endl;
		}
		drift_file.close();
		m_ntp_server_state.m_drifts.last_time_backup = std::chrono::steady_clock::now();
	}

	if (std::chrono::minutes(1) >= m_ntp_server_state.m_drifts.time_from_last_calc())
		return;

	struct drift_entry {
		time_point_t client_time_ns;
		double adjusted_offset_ns;
	};
	std::vector<drift_entry> temp_drift_table;
	for (const auto& drift_record : m_ntp_server_state.m_drifts.m_drift_table)
		temp_drift_table.push_back(drift_entry{ drift_record.first, drift_record.second });


	// Calculate local clock drift rate based on drift table
	double total_drift_rate_ns_per_sec = 0;
	for (size_t i = 1; i < temp_drift_table.size(); ++i) {
		const auto& entry1 = temp_drift_table[i - 1];
		const auto& entry2 = temp_drift_table[i];
		const auto delta_time_ns = entry2.client_time_ns - entry1.client_time_ns;
		const double delta_delta_ns = entry2.adjusted_offset_ns - entry1.adjusted_offset_ns;

		// Calculate the drift rate and accumulate it
		double drift_rate_ns_per_sec = delta_delta_ns / ntp::seconds_fp(delta_time_ns).count();
		total_drift_rate_ns_per_sec += drift_rate_ns_per_sec;
	}

	// Apply drift adjustment (positive: local clock is slower, negative: local clock is faster)
	const double avg_ns_drift_per_second = total_drift_rate_ns_per_sec / (temp_drift_table.size() - 1); // Avoid division by zero

	m_ntp_server_state.m_drifts.last_time_calced = std::chrono::steady_clock::now();
	m_ntp_server_state.m_drifts.m_drift_rate = avg_ns_drift_per_second / (1.0e+9); //making it fucntion of time
}

bool ntp::Client::is_fine_tunning() const {
	return has_time_private() && !is_fine_tunning_over();
}

bool ntp::Client::is_fine_tunning_over() const {

	if (!has_time_private())
	{
		if ((std::chrono::steady_clock::now() - m_ntp_server_state.m_server_com_init_start_time) > m_settings.fta.max_com_establish_duration) {
			m_stats.m_finetune_over = FineTuneOverReason::EstablishingComTimedout;
			return true;
		}
		else
			return true;
	}

	if (m_ntp_server_state.m_fine_tune_procedure_over) return true;

	if (0 > m_settings.fta.fine_adjustment_duration.count()) { //feature is disabled by the user?
		m_stats.m_finetune_over = FineTuneOverReason::FinetuneDisabled;
		return (m_ntp_server_state.m_fine_tune_procedure_over = true);
	}
	if (0 != m_settings.fta.nb_queries && m_stats.m_nb_server_calls >= m_settings.fta.nb_queries) {//Required queries executed ?
		m_stats.m_finetune_over = FineTuneOverReason::QueriesMaxedOut;
		return (m_ntp_server_state.m_fine_tune_procedure_over = true);
	}
	if (//target clock adjustemt reached ?
		m_ntp_server_state.m_clock_update_result->m_mtr.ds.has_stats() &&
		m_settings.fta.clock_adjustment_jitter_min >= ntp::nanoseconds_fp(m_ntp_server_state.m_clock_update_result->m_mtr.ds.clock_adjustment_jitter_ns)
		) {
		m_stats.m_finetune_over = FineTuneOverReason::ReachedTargetClockAdjustment;
		return (m_ntp_server_state.m_fine_tune_procedure_over = true);
	}
	if (//feature time duration is over
		0 != m_settings.fta.fine_adjustment_duration.count() &&
		m_settings.fta.fine_adjustment_duration < std::chrono::nanoseconds(std::chrono::steady_clock::now() - m_ntp_server_state.m_fine_tune_procedure_start_time)
		) {
		m_stats.m_finetune_over = FineTuneOverReason::FineTuningTimedout;
		return (m_ntp_server_state.m_fine_tune_procedure_over = true);
	}

	return false;
}

ResultPtr ntp::Client::last_clock_update_query_result() const
{
	ResultPtr ptr;
	{
		std::shared_lock<std::shared_mutex> clock_lock(m_clock_lock);
		ptr = m_ntp_server_state.m_clock_update_result;
	}
	return ptr;
}

EndPointPtr ntp::Client::queried_server_endpoint() const { return m_ntp_server_state.m_ep; }

double ntp::Client::server_queries_success_rate() const {
	return m_stats.has_queries_success_rate() ? m_stats.m_queries_success_rate : 0;
}

std::chrono::nanoseconds ntp::Client::duration_from_last_clock_update() const
{
	std::shared_lock<std::shared_mutex> clock_lock(m_clock_lock);
	if (!has_time_private()) return std::chrono::nanoseconds(-1);
	return std::chrono::nanoseconds(std::chrono::steady_clock::now() - m_ntp_server_state.m_prev_clock_update_time);
}

std::chrono::nanoseconds ntp::Client::duration_from_successful_query() const
{
	std::shared_lock<std::shared_mutex> clock_lock(m_clock_lock);
	if (!has_time_private()) return std::chrono::nanoseconds(-1);
	return std::chrono::nanoseconds(std::chrono::steady_clock::now() - m_stats.m_prev_sucessful_query_time);
}

bool ntp::Client::set_system_clock(const ResultPtr res) const
{
	if (!res || !res->has_time())
		return false;

	return ntp::utils::SetSystemClockResult::Success == ntp::utils::set_system_clock(res->now().time());
}

bool ntp::Client::has_time() const {
	return has_time_private();
}

ntp::Result::Result() :
	m_status(Status::UnknownErr)
	, m_time(time_point_t::min())
	, m_extraction_time(std::chrono::steady_clock::time_point::min())
	, m_used_for_setting_clock(false)
	, m_server_ep(nullptr)
{
	memset(&m_mtr, 0, sizeof(m_mtr));
	m_mtr.ds.clear();
}

std::string ntp::Result::as_string() const
{
	std::stringstream ss;
	if (server_info())
		ss << "server: " << server_info()->as_string();
	else
		ss << "Server: " << server()->as_string();

	ss << std::endl << "Result state: " << StatusToString(m_status);

	if (m_used_for_setting_clock)
		ss << "\t\t[TIME UPDATED]";
	const auto t = time();
	if (!t)
		ss << std::endl << "Time: N/A";
	else {
		ss << std::endl << TimeResultToString(t, now(), m_mtr);
	}
	return ss.str();
}

ntp::DateTime ntp::Result::now() const {
	std::chrono::nanoseconds dur_from_ref(std::chrono::steady_clock::now() - m_extraction_time);
	dur_from_ref = dur_from_ref + ntp::utils::steady_clock_accumulated_drift(m_mtr.drift_rate, dur_from_ref);
	return !has_time() ? ntp::DateTime() : ntp::DateTime(m_time + dur_from_ref);
}

void ntp::Client::stats::clear()
{
	m_finetune_over = FineTuneOverReason::None;
	m_prev_sucessful_query_time = std::chrono::steady_clock::time_point::min();
	m_nb_stats_server_calls = m_nb_server_msg_transmission_errors = m_nb_server_calls = 0;
	m_queries_success_rate = std::numeric_limits<double>::signaling_NaN();
	m_network_latency.clear();
	m_clock_deltas.clear();
}

void ntp::Client::ServerState::clear()
{
	m_ep.reset();
	m_ep_ip.reset();
	m_delta_with_drift_ns = m_offset_ns = m_delay_ns = m_delta_no_drift_ns = m_drift_ns = 0;
	m_fine_tune_procedure_over = false;
	m_time = time_point_t::min();
	m_fine_tune_procedure_start_time = m_prev_clock_update_time = std::chrono::steady_clock::time_point::min();
	m_server_com_init_start_time = std::chrono::steady_clock::time_point::min();
	m_clock_update_result.reset();
	m_last_result_mtr.reset();
	m_drifts.clear();
	m_sd.clear();
}

ntp::Client::ServerState::ServerState() { clear(); }

std::chrono::seconds ntp::Client::ServerState::drifts_data::time_from_last_calc() const {
	if (std::chrono::steady_clock::time_point::min() == last_time_calced)
		return std::chrono::seconds::max();

	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_time_calced);
}

std::chrono::seconds ntp::Client::ServerState::drifts_data::time_from_last_backup() const
{
	if (std::chrono::steady_clock::time_point::min() == last_time_backup)
		return std::chrono::seconds::max();

	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_time_backup);
}

void ntp::Client::ServerState::drifts_data::clear()
{
	m_drift_table.clear();
	m_drift_rate = 0;
	last_time_backup = last_time_calced = std::chrono::steady_clock::time_point::min();
}
