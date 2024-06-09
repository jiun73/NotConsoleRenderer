#ifndef __ntp_h__
#define __ntp_h__

#include <stdint.h>
#include <chrono>
#include <iomanip>
#include <assert.h>
#include <cmath>
#include <cstring>//for std::memset
#include "include/intp.h"

/*
Below is a description of the NTP / SNTP Version 4 message format,
which follows the IPand UDP headers.This format is identical to
that described in RFC - 1305, with the exception of the contents of the
reference identifier field.The header fields are defined as follows :

1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| LI | VN | Mode | Stratum | Poll | Precision |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Root Delay |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Root Dispersion |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Reference Identifier |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Reference Timestamp(64)                     |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Originate Timestamp(64)                     |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                    Receive Timestamp(64)                      |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                    Transmit Timestamp(64)                     |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Key Identifier(optional) (32) |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                                                               |
|                 Message Digest(optional) (128)                |
|                                                               |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Reference Timestamp : This is the time at which the local clock was
last set or corrected, in 64 - bit timestamp format.

Originate Timestamp : This is the time at which the request departed
the client for the server, in 64 - bit timestamp format.

Receive Timestamp : This is the time at which the request arrived at
the server, in 64 - bit timestamp format.

Transmit Timestamp : This is the time at which the reply departed the
server for the client, in 64 - bit timestamp format.


 * NTP uses two fixed point formats.  The first (l_fp) is the "long"
 * format and is 64 bits long with the decimal between bits 31 and 32.
 * This is used for time stamps in the NTP packet header (in network
 * byte order) and for internal computations of offsets (in local host
 * byte order). We use the same structure for both signed and unsigned
 * values, which is a big hack but saves rewriting all the operators
 * twice. Just to confuse this, we also sometimes just carry the
 * fractional part in calculations, in both signed and unsigned forms.
 * Anyway, an l_fp looks like:
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                         Integral Part                         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                         Fractional Part                       |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * REF http://www.eecis.udel.edu/~mills/database/rfc/rfc2030.txt

	offset = [(T2 - T1) + (T3 - T4)] / 2
	delay  =  (T4 - T1) - (T3 - T2)

 */


#define NTP_PACKET_LEN							48

//see https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-sntp/3c344366-f8ad-44f9-adf6-0f636f1cdeac
//and https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-sntp/3c344366-f8ad-44f9-adf6-0f636f1cdeac
//to support AUTHENTICATED packets in the future.

#define NTP_PACKET_AUTHENTICATED_LEN			68
#define NTP_PACKET_EXT_AUTHENTICATED_LEN		120
		
#define ROUND_AND_TRUNC 0//when set we will round/trunc fraction of seconds

namespace ntp
{
	//Mode enumeration:
	/**
	 * This value indicates the mode, with values defined as follows:
	 *
	 * Mode     Meaning
	 * ----     -------
	 * 0        reserved
	 * 1        symmetric active
	 * 2        symmetric passive
	 * 3        client
	 * 4        server
	 * 5        broadcast
	 * 6        reserved for NTP control message
	 * 7        reserved for private use
	 *
	 * In unicast and anycast modes, the client sets this field to 3 (client)
	 * in the request and the server sets it to 4 (server) in the reply. In
	 * multicast mode, the server sets this field to 5 (broadcast).
	 */
	enum class AssociationModes : uint8_t {
		Reserved = 0,         // reserved
		SymActive,            // symmetric active
		SymPassive,           // symmetric passive
		Client,               // client
		Server,               // server
		Broadcast,            // broadcast
		ControlMessage,       // NTP control message
		ReservedPrivate,      // reserved for private use
	};

	struct NtpPacket			 // Total: 384 bits or 48 bytes.
	{
		union ReferenceIdentifier
		{
			uint8_t bytes[4];
			char    ascii[4];
		};

		union NtpTimeFormat {
			struct {
				uint32_t s;        // 32 bits. seconds.
				uint32_t f;        // 32 bits. fraction of a second.
			}segments;
			uint64_t all;
		};

		union NtpTimeShortFormat {
			struct {
				uint16_t s;        // 16 bits. seconds.
				uint16_t f;        // 16 bits. fraction of a second.
			}segments;
			uint32_t all;
		};

		uint8_t li_vn_mode;      //  8 bits. li, vn, and mode.
								 //    -li   2 bits. Leap indicator.
								 //    -vn   3 bits. Version number of the protocol. 3,4
								 //    -mode 3 bits. Client will pick mode 3 for client.

		uint8_t  stratum;        //  8 bits. Stratum level of the local clock.


		/**
		* This value indicates the maximum interval between successive messages,
		* in seconds to the nearest power of two. The values that can appear in
		* this field presently range from 4 (16 s) to 14 (16284 s); however, most
		* applications use only the sub-range 6 (64 s) to 10 (1024 s).
		*/
		uint8_t   poll;           //  8 bits. Maximum interval between successive messages.

		/**
		* This value indicates the precision of the local clock, in seconds to
		* the nearest power of two.  The values that normally appear in this field
		* range from -6 for mains-frequency clocks to -20 for microsecond clocks
		* found in some workstations.
		*/
		int8_t   precision;      //  8 bits. Precision of the local clock.


		/**
		* This value indicates the total roundtrip delay to the primary reference
		* source. Note that this field can take on both positive
		* and negative values, depending on the relative time and frequency
		* offsets. The values that normally appear in this field range from
		* negative values of a few milliseconds to positive values of several
		* hundred milliseconds.
		*/
		NtpTimeShortFormat rootDelay;      // 32 bits. Total round trip delay time.


		/**
	    * This value indicates the nominal error relative to the primary reference
	    * The values  that normally appear in this field
	    * range from 0 to several hundred milliseconds.
	    */
		NtpTimeShortFormat rootDispersion; // 32 bits. Max error aloud from primary clock source.

		/**
		* This is a 4-byte array identifying the particular reference source (set by server only).
		* In the case of NTP Version 3 or Version 4 stratum-0 (unspecified) or
		* stratum-1 (primary) servers, this is a four-character ASCII string, left
		* justified and zero padded to 32 bits. For a stratum-0 this string is a KISS code used for debugging and monitoring purposes.
		* For stratum-1 (reference clock) this string is a assigned to the radio clock.
		* In NTP Version 3 & 4 secondary servers, this is the 32-bit IPv4 address of the reference source.
		* NTP primary (stratum 1) servers should set this field to a code identifying the external
		* reference source according to the following list. If the external
		* reference is one of those listed, the associated code should be used.
		* Codes for sources not listed can be contrived as appropriate.
		*
		* Code     External Reference Source
		* ----     -------------------------
		* LOCL     uncalibrated local clock used as a primary reference for
		*          a subnet without external means of synchronization
		* PPS      atomic clock or other pulse-per-second source
		*          individually calibrated to national standards
		* ACTS     NIST dialup modem service
		* USNO     USNO modem service
		* PTB      PTB (Germany) modem service
		* TDF      Allouis (France) Radio 164 kHz
		* DCF      Mainflingen (Germany) Radio 77.5 kHz
		* MSF      Rugby (UK) Radio 60 kHz
		* WWV      Ft. Collins (US) Radio 2.5, 5, 10, 15, 20 MHz
		* WWVB     Boulder (US) Radio 60 kHz
		* WWVH     Kaui Hawaii (US) Radio 2.5, 5, 10, 15 MHz
		* CHU      Ottawa (Canada) Radio 3330, 7335, 14670 kHz
		* LORC     LORAN-C radionavigation system
		* OMEG     OMEGA radionavigation system
		* GPS      Global Positioning Service
		* GOES     Geostationary Orbit Environment Satellite*/
		
		ReferenceIdentifier refId;// 32 bits. Reference clock identifier.		

		NtpTimeFormat refTm;	 //Reference time-stamp.

		NtpTimeFormat origTm;	 //Originate time-stamp.= t1

		NtpTimeFormat rxTm;		 //Received time-stamp. = t2

		NtpTimeFormat txTm;		 //Transmit time-stamp. = t3
	};

	struct NtpAuthenticatedPacket : NtpPacket {
		uint8_t authenticator[8];// (optional)(160 bits)
	};

	struct NtpExtendedAuthenticatedPacket : NtpPacket {
		uint8_t authenticator[72];// (optional) (576 bits)
	};

	union NtpPacketBuffer{
		NtpPacket pkt;
		char as_buffer[sizeof(NtpPacket)];
	};

	static_assert(sizeof(ntp::NtpPacket) == NTP_PACKET_LEN, "size of NtpPacket is unexpected, check your alignemen and padding settings.");
	static_assert(sizeof(NtpPacketBuffer) == sizeof(NtpPacket) && sizeof(NtpPacket) == NTP_PACKET_LEN, "size of NtpPacket is unexpected, check your alignemen and padding settings.");

	//Defines NTP era(epoc) as a signed counter, where the era=0 is the 1st epoc that starts on 1/1/1900.
	//when ever the era rollsover (see [NTP_ERA_SEC]) the counter increments.
	typedef int32_t NtpEra;
	static const NtpEra ntp_era_auto = std::numeric_limits<NtpEra>::min();

	//Amount of seconds in NTP era(epoch) which are about 136 years. Every 2^32 seconds the timestamp rollover and starts again,
	//as the era is incremented (the 1st era is 0 on 1/1/1900).
	//its equivilant to 2^32
	constexpr int64_t NTP_ERA_SEC(1LL<<32); 

	//Delta between ntp time (seconds from 01/01/1900) and posix time (seconds from 01/01/1970)
	constexpr int64_t NTP_TS_DELTA(2208988800LL);

	//When a server determines that a client is behaving inappropriately, it includes a specific KoD value in its response packet
	//to signal the client to back off.
	//Kiss of Death (KoD) code is stored in 32 bytes (4 characters),
	//in the "Reference Identifier" field. One should be looked for if the server's flags the respose with
	//AssociationModes::ControlMessage.
	namespace KoD {
		//(Rate exceeded) : The client is sending requests too frequently, and the server wants it to reduce the rate of requests.
		static const char RateExceeded[] = { "RATE" };
		//(Access denied) : The server refuses to provide time synchronization to the client for security or policy reasons.
		static const char AccessDenied[] = { "DENY" };
		//(Access denied - response required) : Similar to "DENY," but the client is required to respond to the server before further requests are accepted.
		static const char AccessDenied_ResponseRequired[] = { "RSTR" };
		enum class KoD{None, RateExceeded, AccessDenied, AccessDenied_ResponseRequired };
	};

	//Parses the ntp packet precision field, and returns the accuracy specefied.
	static inline std::chrono::nanoseconds parse_precision(int8_t ntp_precision_field)	{
		const double precisionSeconds = std::pow(2.0, static_cast<double>(ntp_precision_field));
		return std::chrono::duration_cast<std::chrono::nanoseconds>(ntp::seconds_fp(precisionSeconds));
	}

	//calc NTP server clock pecision calue in NTP format (2^(-10) sec)
	static inline int8_t calculate_precision(std::chrono::nanoseconds precision) {
		const double seconds_precision = std::chrono::duration_cast<ntp::seconds_fp>(precision).count();
		// NTP precision is stored in sighed 8bits integer
		return static_cast<int8_t>(0.5 + std::log2(seconds_precision));
	}	

	//The "poll" value is an important part of the NTP packet, and it represents the maximum interval between successive polls in seconds.
	//In the context of an NTP server,
	//the poll value indicates how often clients should contact the server to synchronize their time.
	//A smaller poll interval means clients will synchronize more frequently, while a larger poll interval means less frequent synchronization.
	//For example, if the poll value is set to 6, then the actual poll interval is calculated as 2 ^ 6 = 64 seconds.
	//Similarly, if the poll value is set to 4, the poll interval would be 2 ^ 4 = 16 seconds.
	static inline uint8_t calculate_clients_polling_min_intervals(int minimal_intervals_seconds){
		if (0 >= minimal_intervals_seconds)
			minimal_intervals_seconds = 1;

		return static_cast<uint8_t>(0.5 + std::log2(minimal_intervals_seconds));
	}

	//returns the NTP version from li_vn_mode byte.
	static inline ntp::ProtocolVersion get_ntp_version(uint8_t li_vn_mode) {
		static const uint8_t verion_mask = 0x38;//00111000;
		const uint8_t client_ntp_version = (li_vn_mode & verion_mask) >> 3;
		return ntp::ProtocolVersion(client_ntp_version);
	}
	
	//Returns a string representation of a reference identifier according to the rules set out in RFC 2030.
	//This function should be called WITHOUT translating the refid to network order 
	std::string parse_reference_identifier(const ntp::NtpPacket::ReferenceIdentifier& refid, ntp::StratumLevel stratum, ntp::ProtocolVersion ver, ntp::AssociationModes assoc_mode);

	//returns the NTP mode from li_vn_mode byte.
	static inline AssociationModes get_ntp_mode(uint8_t li_vn_mode) { return static_cast<AssociationModes>(li_vn_mode & 0x7); }

	//returns the leap indicator from li_vn_mode byte.
	static inline LeapIndicator get_ntp_leap(uint8_t li_vn_mode) { return static_cast<LeapIndicator>((li_vn_mode & 0xC0) >> 6); }

	//returns li_vn_mode byte for client/or server
	static inline uint8_t create_li_vn_mode(AssociationModes mode, LeapIndicator li, ntp::ProtocolVersion ntp_ver)
	{
		const uint8_t ver = uint8_t(ntp_ver) << 3;
		const uint8_t leap_year_flag = uint8_t(li)<<6;
		return leap_year_flag | ver | uint8_t(mode);

		//break it down step by step:

		// 8 bits. li, vn, and mode.
		//    -li   2 bits. Leap indicator.
		//    -vn   3 bits. Version number of the protocol. 3,4
		//    -mode 3 bits.

		//(client_li_vn_mode & 0x38):  This bitwise AND operation is used to preserve the existing bits of the first byte in the clinet request(client_li_vn_mode)
		//							   except for the bits related to the Leap Indicator, which are bits 6 and 7 (it will aso clear the mode keeping only the version bits).
		// 		                       The value 0x38 (or 00111000 in binary) is a bitmask that preserves all the bits except bits 6 and 7.
		//(4 << 3):				       This bit shift operation shifts the value (for example  4) left by 3 positions, which effectively places the value 4 in bits 5, 4, 3, and 2 of the first byte.
		//							   This corresponds to setting the Version Number field to 4 (which represents NTPv4).
		//AssociationModes:			   This is the Mode value.The Mode field specifies the purpose of the packet.
		//							   In this case, 4 represents the Server mode, indicating that the packet is coming from an NTP server.

		//Combining these operations, the code sets the Leap Indicator to its current value, sets the Version Number to 4 (NTPv4), and sets the Mode to 4 (Server mode).
		//This ensures that the NTP response packet conforms to the required specifications of the NTP protocol.
		//Remember, the first byte of the NTP packet is a bit - packed field where different bits represent different pieces of information.
		//By manipulating these bits, you control various aspects of the NTP packet, such as the Leap Indicator, Version Number, and Mode.
	}

	int64_t get_posix_time_microseconds();
	
	//returns NTP duration format from std::chrono duration.
	//seconds, fractions: seconds and fraction of seconds in ntp format.
	//					  normaly stored in [uint32_t] data type.
	//
	//notes:
	//1. There is an inherint percision flaw when working in interger storage for duration here.
	//	 its recommanded to prefer floating points storage such as [ntp::seconds_fp] instead of [std::chrono::milliseconds]
	//	 or [std::chrono::michroseconds]
	//
	//2. Works also for non zero NTP Eras, the seconds will be trimmed to accomodate the next epoc.
	//   for uint32_t example: NTP date [4294944000 seconds] (era/epoc 0) will get the ntp timestamp [4294944000 seconds]
	//	 for uint32_t example: NTP date [4295030400 seconds] (era/epoc 1) will get the ntp timestamp [63104 seconds]
	//	 for uint32_t example: NTP date [34712668800 seconds] (era/epoc 8) will get the ntp timestamp [352930432 seconds]
	template<typename T, class StdChronoDuation>
	static inline void get_ntp_duration_from_chrono_duration(T &seconds, T &fractions, const StdChronoDuation &std_chrono_dur)
	{
		const uint64_t MAX_TS_FRACTION = 1ULL << (8*sizeof(T));

		//The line below allows NTP eras compatability by trimming out values not in range of [MAX_TS_FRACTION]
		seconds = static_cast<T>(std::chrono::duration_cast<std::chrono::seconds>(std_chrono_dur).count());		
		const auto sec_fractions = ntp::seconds_fp(std_chrono_dur - std::chrono::seconds(seconds));
		#if(ROUND_AND_TRUNC) 
				fractions = static_cast<T>(0.5 + sec_fractions.count() * MAX_TS_FRACTION);
		#else
				fractions = static_cast<T>(sec_fractions.count() * MAX_TS_FRACTION);
		#endif//ROUND_AND_TRUNC
	}

	//returns std::chrono duration from NTP duration format.
	//seconds, fractions: seconds and srcaction of seconds in posix format.
	//					  normaly stored in [uint32_t] data type.
	//
	//note: There is an inherint percision flaw when working in interger storage for duration here.
	//		its recommanded to prefer floating points storage such as [ntp::seconds_fp] instead of [std::chrono::milliseconds]
	//		or [std::chrono::michroseconds]
	template<typename T, class StdChronoDuation>
	static inline void get_chrono_duration_from_ntp_duration(StdChronoDuation &out_std_chrono_dur, T seconds, T fractions)
	{
		const uint64_t MAX_TS_FRACTION = 1ULL<<(8*sizeof(T));
		out_std_chrono_dur = std::chrono::seconds(seconds) + 
#if(ROUND_AND_TRUNC) 
							 std::chrono::duration_cast<StdChronoDuation>(std::chrono::duration<double, std::ratio<1, MAX_TS_FRACTION>>(0.5+fractions));
#else
							 std::chrono::duration_cast<StdChronoDuation>(std::chrono::duration<T, std::ratio<1, MAX_TS_FRACTION>>(fractions));
#endif//ROUND_AND_TRUNC
	}	

	//converts posix time to ntp time.
	//seconds, fractions: seconds and fraction of seconds in ntp format.
	//					  normaly stored in [uint32_t] data type.
	//
	//notes:
	//1. There is an inherint percision flaw when working in interger storage for duration here.
	//   its recommanded to prefer floating points storage such as [ntp::seconds_fp] instead of [std::chrono::milliseconds]
	//   or [std::chrono::michroseconds]
	//
	//2. Works also for non zero NTP Eras, the seconds will be trimmed to accomodate the next epoc.
	//   for uint32_t example: NTP date [4294944000 seconds] (era/epoc 0) will get the ntp timestamp [4294944000 seconds]
	//	 for uint32_t example: NTP date [4295030400 seconds] (era/epoc 1) will get the ntp timestamp [63104 seconds]
	//	 for uint32_t example: NTP date [34712668800 seconds] (era/epoc 8) will get the ntp timestamp [352930432 seconds]
	template<typename T, class StdChronoDuation>
	static inline void ntp_time_from_posix(T &seconds, T &fractions, const StdChronoDuation& posix)
	{
										 //	NTP_TS_DELTA is added to treate it as NTP time not as posix
		const NtpEra detected_ntp_era = NtpEra((NTP_TS_DELTA + std::chrono::duration_cast<std::chrono::seconds>(posix).count()) / NTP_ERA_SEC);

		get_ntp_duration_from_chrono_duration(seconds, fractions, posix);
		seconds = seconds + NTP_TS_DELTA; //convert the posix time seconds to seconds from 01 / 01 / 1900
	}

	//NTP timestamp to posix microsc
	//seconds, fractions: seconds and fraction of seconds in ntp format.
	//					  normaly stored in [uint32_t] data type.
	//
	//note: There is an inherint percision flaw when working in interger storage for duration here.
	//		its recommanded to prefer floating points storage such as [ntp::seconds_fp] instead of [std::chrono::milliseconds]
	//		or [std::chrono::michroseconds]
	template<typename T, class StdChronoDuation>
	static void ntp_time_to_posix(StdChronoDuation& out_posix, T seconds, T fractions, NtpEra ntp_era = ntp_era_auto)	{
		//convert ntp time (seconds from 01/01/1900) to posix time (seconds from 01/01/1970) by substructing NTP_TS_DELTA

		//In order to avoid overflow when substracting [NTP_TS_DELTA] from lower NTP seconds value (for example right after new NTP epocs)
		//We may substruct the [NTP_TS_DELTA] after the conversion.
		if (seconds >= static_cast<T>(NTP_TS_DELTA)) {
			get_chrono_duration_from_ntp_duration<T>(out_posix, seconds - static_cast<T>(NTP_TS_DELTA), fractions);
		}
		else {
			get_chrono_duration_from_ntp_duration<T>(out_posix, seconds, fractions);
			out_posix -= std::chrono::seconds(NTP_TS_DELTA);
		}

		if (ntp_era_auto == ntp_era)
		{
			//Method 1: assume the current systme clock is not way off the "real" time, and both are on the same era:
			std::chrono::seconds posix_now_secs;
			ntp::DateTime(std::chrono::system_clock::now()).as_posix_time(posix_now_secs);
			ntp_era = NtpEra((posix_now_secs.count() + NTP_TS_DELTA)/NTP_ERA_SEC);
			
#if 0
			//Metod 2: //not reling ng on the system clock and supporting only the part of the 1st epoc and part on the 2nd epoc
			//specificly 136 years starting from December 27 2003 08:58 AM
			//detecting era rollover:
			const T DEC_27_2023_08_58_AM_NTP_SEC(3912649069);
			ntp_era = (DEC_27_2023_08_58_AM_NTP_SEC > seconds) ? 1 : 0;			
#endif
		}
				
		if(0 != ntp_era)
			out_posix += std::chrono::seconds(NTP_ERA_SEC*ntp_era);
	}		

	//Breakes down ntp::time_point_t to date-time components.
	//The results are eather in local time or in UTC (see extract_as_local argument)
	bool extract_time_point(bool extract_as_local, const ntp::time_point_t& tp, int& y, int& m, int& d, std::chrono::hours& hr, std::chrono::minutes& min, std::chrono::seconds& sec, std::chrono::milliseconds& ms, std::chrono::microseconds& us, std::chrono::nanoseconds& ns);

	//extract_as_local: whether to get the [DateTimePt] as local or as universal time
	inline static void time_pt_class_to_struct(bool extract_as_local, const time_point_t &tp, DateTimePt& time_pt) {
		if (extract_time_point(extract_as_local, tp, time_pt.year, time_pt.mon, time_pt.mday, time_pt.hour, time_pt.min, time_pt.sec, time_pt.msec, time_pt.usec, time_pt.nsec))
			time_pt.tm_kind = extract_as_local ? DateTimePt::Kind::Local : DateTimePt::Kind::UTC;
		else {
			std::memset(&time_pt, 0, sizeof(time_pt));
			time_pt.tm_kind = DateTimePt::Kind::Invalid;
		}
	}

}//end of namespace ntp

#endif// __ntp_h__
