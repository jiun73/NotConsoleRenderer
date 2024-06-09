#ifndef __ntp_utils_h__
#define __ntp_utils_h__

#include <chrono>
#include <vector>
#include <string>
#include <stdint.h>
#include "ntp.h"

namespace ntp {
namespace utils {

	//Turns on/off accurate time mechanizem for the process (needs to be turned off after).
	void switch_os_accurate_clock(bool switch_on);	

	struct ping_result {
		std::vector<std::chrono::nanoseconds> roundtrip;
		std::chrono::nanoseconds median_roundtrip;
		std::chrono::nanoseconds max_roundtrip;
		std::chrono::nanoseconds min_roundtrip;
		std::chrono::nanoseconds avg_roundtrip;
		bool sucess = false;
	};

	//attempts to perform icmp echo (ping) on the give host/url
	bool ping(std::string hostname, uint16_t nb_pings, ping_result& out_res, std::chrono::milliseconds timeout = std::chrono::seconds(3), std::chrono::nanoseconds requests_intervals = std::chrono::nanoseconds(0));

	//attempts to perform ping like command specificly targeting NTP SERVERS. this will be done by sending ntp requests and using the result's latency.
	bool ping_ntp_server(ntp::EndPointPtr server_endpoint, uint16_t nb_pings, ping_result& out_res, std::chrono::milliseconds timeout = std::chrono::seconds(3), std::chrono::nanoseconds requests_intervals = std::chrono::nanoseconds(0));

	//customized version of sprintf that dynamicly alocates the required buffers.
	std::string format_string(const char* format, ...);

	//sets the system clock
	enum class SetSystemClockResult {Success, Failed, Failed_AdminRightsNeeded};
	SetSystemClockResult set_system_clock(const ntp::time_point_t& tm);
	static inline SetSystemClockResult set_system_clock(const ntp::DateTime& t) {return set_system_clock(t.time());}
	static inline SetSystemClockResult set_system_clock(const ntp::ManagedClockPtr mc) {
		return (!mc || !mc->has_time()) ? SetSystemClockResult::Failed : set_system_clock(mc->now());
	}
	//returns the accumilated steady_clock drift given a duration mesured with [std::chrono::steady_clock] and a drift rate.
	//Note that the returned drift and be nagative
	static inline std::chrono::nanoseconds steady_clock_accumulated_drift(double drift_rate, const std::chrono::nanoseconds& steady_clock_duration) {
		return std::chrono::nanoseconds(static_cast<int64_t>(steady_clock_duration.count()*drift_rate));
	}

	//returns the accumilated steady_clock drift given a duration mesured with [std::chrono::steady_clock] and a drift rate.
	//Note that the returned drift and be nagative
	static inline nanoseconds_fp steady_clock_accumulated_drift(double drift_rate, const nanoseconds_fp& steady_clock_duration) {
		return nanoseconds_fp(steady_clock_duration.count()*drift_rate);
	}

	//returns the accumilated steady_clock drift of (std::chrono::steady_clock::now() - ref_time) and a drift rate.
	//Note that the returned drift and be nagative
	static inline std::chrono::nanoseconds steady_clock_accumulated_drift(double drift_rate, const std::chrono::steady_clock::time_point& ref_time) {
		return steady_clock_accumulated_drift(drift_rate, std::chrono::steady_clock::now() - ref_time);
	}
}}

#endif// __ntp_utils_h__