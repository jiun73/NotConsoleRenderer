#ifndef __intp_h__
#define __intp_h__

#include <stdint.h>
#include <string>
#include <memory>
#include <chrono>

namespace ntp 
{
	const uint16_t UDP_PORT = 123;

	//should be called before starting work on the ntp library.
	void init();

	/**
	* This value indicates the NTP/SNTP version number.  The version number
	* is 3 for Version 3 (IPv4 only) and 4 for Version 4 (IPv4, IPv6 and OSI).*/
	enum class ProtocolVersion : uint8_t {
		V3 = 3,//Version 3 (IPv4 only)
		V4 = 4,//Version 4 (IPv4, IPv6 and OSI).
	};

	//Warning indicator comming from the (originaly stored on 2 bits comming from on the server's respose).
	enum class LeapIndicator {

		//No warnnings.
		NoWarning = 0,

		/*
		When the last second of the month has a leap has impending leap second (to be inserted or deleted)
		leap indicator of [1] or [2] will be set at the last day of the month prompting that the last minute of the day.
		at 23:59:00 UTC, should be slowed down (LastMinuteOfTheDayLeapSecondInsertion) gradualy to accommodate for the earth rotation delta from 24h.
		NTP specification theoretically allows for negative leap indicators (speed up) but its extreamly unlikly that earth rotation will speed up.
		This will take place evry few years based on the "International Earth Rotation and Reference Systems Service" (IERS).
		When it takes place its usualy at the end of June or December, and single second is added, only rarely a second is subtructed.
		*/

		//last minute of the day has 61 second.
		//This signals that a leap second will be inserted at the end of the current day(23:59 : 60 UTC).
		//This allows devices to prepare for the upcoming leap second and adjust their clocks accordingly.
		LastMinuteOfTheDayLeapSecondInsertion,

		//This indicates that a leap second has been inserted within the past 48 hours.
		//This informs devices that they need to adjust their clocks by adding one second to compensate for the inserted leap second.
		//If thy getting their clock info from an ntp server, no oeration should take place (the clock will already be ajusted).
		LeapSecondInserted,

		//server clock is out of synch
		NotSynched,
	};

	/*
	In NTP protocol, the Stratum level enumeration values are used to describe how many NTP hops away a machine is from an authoritative time source.
	A stratum 1 time server has a radio or atomic clock directly attached to it. It then sends its time to a stratum 2 time server through NTP, and so on.
	The lower the stratum number or level,	the higher the accuracy of the system clock(s) at that level.
	*/
	enum class StratumLevel : uint8_t {
		Unspecified = 0, //unspecified or invalid
		PrimaryReference = 1, //primary server (e.g., equipped with a GPS receiver)
		SecondaryReference01 = 2, //secondary server (via NTP) 2-15
		SecondaryReference02, SecondaryReference03, SecondaryReference04, SecondaryReference05, SecondaryReference06, SecondaryReference07,
		SecondaryReference08, SecondaryReference09, SecondaryReference10, SecondaryReference11, SecondaryReference12, SecondaryReference13,
		SecondaryReference14, SecondaryReference15,
		Unsynchronized = 16, //unsynchronized 
		Reserved = 17, //reserved 17-255 
		//...
	};

	struct IPAddress {
		enum class Version {V4, V6};
		Version ver;
		union{
			uint8_t v4[4];
			uint8_t v6[16];
		}addr;
	};
	typedef std::shared_ptr<IPAddress> IPAddressPtr;

	enum class SpecialAddress						{ LOOPBACK_IPV4, LOOPBACK_IPV6, ADDR_ANY_IPV4, ADDR_ANY_IPV6 };
	static const char* const SpecialAddressString[] { "127.0.0.1",		"::1",		  "0.0.0.0",		"::"	 };

	struct EndPointUDP {
		const std::string host;
		const uint16_t port;

		explicit EndPointUDP(std::string _host, uint16_t _port = ntp::UDP_PORT) :
			host(_host), port(_port) {}

		explicit EndPointUDP(SpecialAddress _addr, uint16_t _port = ntp::UDP_PORT) :
			host(SpecialAddressString[static_cast<int>(_addr)]), port(_port) {}

		EndPointUDP() :
			host(std::string()), port(ntp::UDP_PORT) {}

		bool empty() const { return host.empty(); }
		std::string as_string() const { return empty() ? "Empty endpoint" : (host + ':' + std::to_string(port)); }

		bool operator == (const EndPointUDP& other) const { return port == other.port && host == other.host; }
		bool operator != (const EndPointUDP& other) const { return !(*this == other); }
	};
	typedef std::shared_ptr<EndPointUDP> EndPointPtr;

	typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point_t;
	typedef std::chrono::duration<double, std::chrono::nanoseconds::period> nanoseconds_fp;
	typedef std::chrono::duration<double, std::chrono::microseconds::period> microseconds_fp;
	typedef std::chrono::duration<double, std::chrono::milliseconds::period> milliseconds_fp;
	typedef std::chrono::duration<double, std::chrono::seconds::period> seconds_fp;


	struct DateTimePt
	{
		enum class Kind { Local, UTC, Invalid };
		DateTimePt::Kind tm_kind = DateTimePt::Kind::Invalid;
		std::chrono::nanoseconds  nsec;	/* nano seconds, range 0 to 999     */
		std::chrono::microseconds usec;	/* micro seconds, range 0 to 999    */
		std::chrono::milliseconds msec;	/* mili seconds, range 0 to 999     */
		std::chrono::seconds	  sec;	/* seconds, range 0 to 59           */
		std::chrono::minutes	  min;	/* minutes, range 0 to 59           */
		std::chrono::hours		  hour;	/* hours, range 0 to 23             */

		int						  mday;	/* day of the month, range 1 to 31  */
		int						  mon;	/* month, range 0 to 11             */
		int						  year;	/* The number of years since 1900   */

		//Returns string represenattion.
		std::string as_string() const;
		std::string as_time_of_day_string() const;
		std::string as_date_string() const;
	};

	class DateTime
	{
		//actual time value that was extracted
		ntp::time_point_t m_time;

	public:
		DateTime();
		explicit DateTime(const ntp::time_point_t &time);
		DateTime(const DateTime &other):m_time(other.m_time) {};
		
		//returns true if no time is available
		inline bool empty() const { return ntp::time_point_t::min() == m_time; }
		
		inline operator bool() const { return !empty(); }
		// A simplistic implementation of operator= (see better implementation below)
		DateTime& operator= (const DateTime& other){
			m_time = other.m_time;
			return *this;
		}

		//will return zero memory structure if [empty()] is true, 
		DateTimePt as_simple_time_local() const;

		//will return zero memory structure if [empty()] is true, 
		DateTimePt as_simple_time_utc() const;

		//will return [negative value] if [empty()] is true, 
		//otherwise returns microseconds since 01/Jan/1970 00:00:00;
		inline int64_t as_posix_time_microsec() const {
			return empty() ? -1 : std::chrono::duration_cast<std::chrono::microseconds>(m_time.time_since_epoch()).count();
		}

		//will return [negative value] if [empty()] is true, 
		//otherwise returns nanoseconds since 01/Jan/1970 00:00:00;
		inline int64_t as_posix_time_nanosec() const {
			return empty() ? -1 : std::chrono::nanoseconds(m_time.time_since_epoch()).count();
		}	
		
		//will return [false] if [empty()] is true, 
		//otherwise returns duration since 01/Jan/1970 00:00:00;
		template<class StdChronoDuation>
		inline bool as_posix_time(StdChronoDuation& outDur ) const {
			if (empty()) return false;
			outDur = std::chrono::duration_cast<StdChronoDuation>(m_time.time_since_epoch());
			return true;
		}

		//returns the time in NTP packet format( seconds and fraction of seconds since 1/1/1900) 
		void as_ntp_time(uint32_t &seconds, uint32_t &fractions) const;

		//Returns the time as string.
		std::string as_local_time_string() const;

		//Returns the time as string.
		std::string as_utc_time_string() const;

		//Returns the time of day as string.
		std::string as_local_time_of_day_string() const;

		//Returns the time of day as string.
		std::string as_utc_time_of_day_string() const;

		//Returns the date as string.
		std::string as_local_date_string() const;

		//Returns the date as string.
		std::string as_utc_date_string() const;

		//Returns the time value
		//will return ntp::time_point_t::min() if [empty()] is true;
		inline ntp::time_point_t time() const { return m_time; }

		//Creating a ntp::DateTime, offseted based on the current time.
		inline ntp::DateTime add_Offset(const std::chrono::nanoseconds& offset) const { return ntp::DateTime(time() + offset); };
	};
	typedef std::shared_ptr<DateTime> TimePtr;

	typedef std::shared_ptr<class ManagedClock> ManagedClockPtr;
	
	//Deccribes a clock created and rund independently from other lcocks.
	class ManagedClock
	{
		//actual time value that was extracted
		ntp::time_point_t m_time;
		//actual time that the time value was extracted.
		std::chrono::steady_clock::time_point m_extraction_time;
		//describes the rate in whitch the std::chrono::steady_clock drifts.
		double m_drift_rate;

	public:
		ManagedClock();
		ManagedClock(const ntp::time_point_t &time, const std::chrono::steady_clock::time_point& extraction_time, double drift_rate);

		inline operator bool() const { return  has_time(); }

		inline bool has_time() const { return ntp::time_point_t::min() != m_time; }

		//Returns the time that the [ManagedClock] was constructed/updated.
		//will return empty if [has_time()] is false;
		inline ntp::DateTime reference_time() const {
			return has_time() ? ntp::DateTime() : ntp::DateTime(m_time);
		}		

		//Returns the current time based on the time extracted.
		//The returned value will be different on each call.
		//will return empty if [has_time()] is false;
		inline ntp::DateTime now() const {
			return !has_time() ? ntp::DateTime() : ntp::DateTime(m_time + duration_from_reference_time());
		}

		//Returns the duration of time passed since the reference time of this clock.
		std::chrono::nanoseconds duration_from_reference_time() const;

		//Creating a new clock, offseted based on the current clock.
		ntp::ManagedClockPtr add_Offset(const std::chrono::nanoseconds& offset) const;
	};


	//Holds extra information about the server.
	struct ServerInfo
	{
		//host name/ip & port of the ntp server
		ntp::EndPointPtr server_ep;

		//The actual resolved address of the server.
		IPAddressPtr server_ip;

		struct Stratum {
			ntp::StratumLevel value;
			std::string as_string() const;
		};

		//stratum the server reported
		Stratum stratum;

		//Protocol version.
		ntp::ProtocolVersion version;

		//Indicator from the server on potential warning.
		ntp::LeapIndicator leap_indicator;

		//The server's [reference_time], i.e the last time the clock on the remote server was last updated/adjusted.
		ntp::DateTime server_reference_time;

		//server's reference clock identification.
		std::string ref_id;

		//The root dispersion reported by the server.
		//If the server gets its time from an external clock, its root dispersion
		//is the estimated maximum error of that clock. If it gets its time from
		//another NTP server, its root dispersion is that server’s root dispersion
		//plus the dispersion added by the network link between them.
		std::chrono::nanoseconds root_dispersion;

		//The root delay reported by the server.
		//The root delay is the round-trip packet delay from ther server to it's upstream servers untill stratum 1 server.
		std::chrono::nanoseconds root_delay;

		//When will hold the minimal query intervals in seconds the server expects.
		//(can change from one request/response to the next).
		std::chrono::seconds min_query_intervals;

		//Average delta from server
		std::chrono::nanoseconds average_delta_abs;
		//Average network latency to server(half of a round trip).
		//this value is not reported by the server, rather it is calculated.
		std::chrono::nanoseconds average_network_latency_abs;		 

		std::string as_string() const;
		ServerInfo();
	};
	typedef std::shared_ptr<ServerInfo> ServerInfoPtr;	

}

#endif// __intp_h__