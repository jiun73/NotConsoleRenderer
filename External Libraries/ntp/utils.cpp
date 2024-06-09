#include "utils.h"
#include "pinger.h"
#include "ntp_client.h"

#include <assert.h>
#include <exception>
#include <sstream>
#include <algorithm>
#include <cstdarg> // Include for std::snprintf
#include <memory>  // Include for std::unique_ptr

#if defined(_MSC_VER) && !defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <timeapi.h>
#endif

#ifndef WIN32
//linux
#include <ctime>
#include <iostream>
#include <sys/time.h>
#endif // WIN32

void ntp::utils::switch_os_accurate_clock(bool switch_on)
{
#ifdef WIN32
#pragma comment(lib, "Winmm.lib")//to call timeGetDevCaps
	TIMECAPS cp = { 0 };
	if (MMSYSERR_NOERROR != timeGetDevCaps(&cp, sizeof(cp))) {
		assert(0);
		throw std::exception();
	}
	if (switch_on)
		timeBeginPeriod(cp.wPeriodMin);//setting the resolution for std::this_thread::sleep
	else
		timeEndPeriod(cp.wPeriodMin);//setting the resolution for std::this_thread::sleep
#endif//WIN32
}

bool ntp::utils::ping(std::string hostname, uint16_t nb_pings, ntp::utils::ping_result& out_res, std::chrono::milliseconds timeout, std::chrono::nanoseconds requests_intervals)
{
	Pinger ping;
	if (!ping.Init(hostname, 255, timeout))
		return false;

	auto res = ping.Start(nb_pings, requests_intervals);
	if (res.empty())
		return false;

	out_res.max_roundtrip = std::chrono::nanoseconds::min();
	out_res.min_roundtrip = std::chrono::nanoseconds::max();
	std::chrono::nanoseconds sum(0);
	std::vector<std::chrono::nanoseconds> sorted;
	for (auto& r : res) {
		if (!r.success)
			continue;
		out_res.roundtrip.push_back(r.round_trip);
		sorted.push_back(r.round_trip);
		sum += r.round_trip;
		out_res.min_roundtrip = std::min(out_res.min_roundtrip, r.round_trip); out_res.max_roundtrip = std::max(out_res.max_roundtrip, r.round_trip);
	}

	if (out_res.roundtrip.empty())
		return false;

	std::sort(sorted.begin(), sorted.end());

	out_res.avg_roundtrip = sum / out_res.roundtrip.size();

	if (2 >= sorted.size())
		out_res.median_roundtrip = out_res.avg_roundtrip;
	else
	{
		if (0 != (sorted.size() % 2))//odd amount, take the middle
			out_res.median_roundtrip = sorted[sorted.size() / 2];
		else//even amount take the avarage to the two in the middle:
			out_res.median_roundtrip = (sorted[sorted.size() / 2] + sorted[sorted.size() / 2 - 1]) / 2;
	}

	out_res.sucess = true;
	return true;
}

bool ntp::utils::ping_ntp_server(ntp::EndPointPtr server_endpoint, uint16_t nb_pings, ntp::utils::ping_result & out_res, std::chrono::milliseconds timeout, std::chrono::nanoseconds requests_intervals)
{
	out_res.max_roundtrip = std::chrono::nanoseconds::min();
	out_res.min_roundtrip = std::chrono::nanoseconds::max();
	std::chrono::nanoseconds sum(0);
	std::vector<std::chrono::nanoseconds> sorted;

	auto ntp_client = ntp::Client::create_instance();
	ntp::ClientSettings cs;
	cs.lts = ntp::LocalTimeSource::InternalClock;
	cs.clock_adjustment_rules.server_response_timeout = timeout;
	cs.clock_adjustment_rules.max_clock_adjustment = std::chrono::nanoseconds::max();//to never get this error;
	ntp_client->init(cs);
	for (uint16_t i = 0; i < nb_pings; ++i) {
		std::this_thread::sleep_for(requests_intervals);
		auto res = ntp_client->query(server_endpoint, false);
		if (!res->has_time())
			continue;

		out_res.roundtrip.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(ntp::nanoseconds_fp(2* res->mtr().delay_ns)));
		sorted.push_back(out_res.roundtrip.back());
		sum += out_res.roundtrip.back();
		out_res.min_roundtrip = std::min(out_res.min_roundtrip, out_res.roundtrip.back()); out_res.max_roundtrip = std::max(out_res.max_roundtrip, out_res.roundtrip.back());
	}

	if (out_res.roundtrip.empty())
		return false;

	std::sort(sorted.begin(), sorted.end());
	out_res.avg_roundtrip = sum / out_res.roundtrip.size();

	if (2 >= sorted.size())
		out_res.median_roundtrip = out_res.avg_roundtrip;
	else
	{
		if (0 != (sorted.size() % 2))//odd amount, take the middle
			out_res.median_roundtrip = sorted[sorted.size() / 2];
		else//even amount take the avarage to the two in the middle:
			out_res.median_roundtrip = (sorted[sorted.size() / 2] + sorted[sorted.size() / 2 - 1]) / 2;
	}

	out_res.sucess = true;
	return true;
}


std::string ntp::utils::format_string(const char* format, ...) {
	// Determine the required buffer size
	va_list args;
	va_start(args, format);
	int size = std::vsnprintf(nullptr, 0, format, args);
	va_end(args);

	// Allocate memory for the formatted string
	std::unique_ptr<char[]> buffer(new char[size + 1]);

	// Format the string using std::snprintf
	va_start(args, format);
	std::vsnprintf(buffer.get(), size + 1, format, args);
	va_end(args);

	// Convert the C-style string to a std::string
	return std::string(buffer.get());
}

ntp::utils::SetSystemClockResult ntp::utils::set_system_clock(const ntp::time_point_t & tm)
{
#ifdef WIN32
	int y, m, d;
	std::chrono::hours h; std::chrono::minutes mm; std::chrono::seconds sec;
	std::chrono::milliseconds ms; std::chrono::microseconds us; std::chrono::nanoseconds ns;
	extract_time_point(true, tm, y, m, d, h, mm, sec, ms, us, ns);
	SYSTEMTIME st;//on Windows OS system time is stored as loacal time.
	st.wYear = (WORD)y;
	st.wMonth = (WORD)m;
	st.wDay = (WORD)d;
	st.wHour = (WORD)h.count();
	st.wMinute = (WORD)mm.count();
	st.wSecond = (WORD)sec.count();
	st.wMilliseconds = (WORD)ms.count();

	auto ret = SetLocalTime(&st);
	if (ret) return SetSystemClockResult::Success;

	auto err = GetLastError();
	if (err == 1314)
		return SetSystemClockResult::Failed_AdminRightsNeeded;
	else
		return SetSystemClockResult::Failed;
#else
	//linux:
	const auto posix_time = std::chrono::duration_cast<std::chrono::microseconds>(tm.time_since_epoch()).count();
	struct timeval tv;
	tv.tv_sec = posix_time / 1000000;
	tv.tv_usec = posix_time % 1000000;
	auto res = settimeofday(&tv, nullptr);
	return (-1 == res) ? SetSystemClockResult::Failed : SetSystemClockResult::Success;
#endif
}
