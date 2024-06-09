#ifndef __intp_client_h__
#define __intp_client_h__

#include <chrono>
#include <thread>
#include <string>
#include <memory>
#include <stdint.h>
#include <cmath>

#include "intp.h"

namespace ntp
{
	struct Metrics
	{ 
		/*delay_ns: delay from transporting packet through network */
		double delay_ns;		   
		/*offset_ns: offset from the server time (not including network latency) */
		double offset_ns;
		/**
		delta_ns: actual difference between server and client time(offset_ns - delay_ns) that should be used for clock adjustments.
				  (this is not to say that the client's internal clock was acrualy adjusted by this diff).*/
		double delta_with_drift_ns;
		/*similar to delta_ns, but includes clock drift correction.*/
		double delta_no_drift_ns;
		//The rate, in which the internal clock is drifting over time.
		double drift_rate;

		//When the internal clinet's clock is not getting updates, this value expresses the delta between the queried result's clock and the internal clock (abs).
		//this only valid for query result that [has_time()] and when the clint also [has_time()]
		double delta_from_internal_clock_ns;
		//The rate, in which the internal clock is drifting over time.

		struct DescriptiveStatistics {

			uint16_t nb_samples;				/*amount of samples collected to calculate the statistics*/

			double network_latency_jitter_ns;	/*network latency jitter or [nan] if not determined */
			double network_latency_avg_ns;		/*average of delay_ns or [nan] if not determined*/

			double clock_adjustment_jitter_ns;	/*clock adjustment (nanoseconds) jitter or [nan] if not determined*/
			double clock_adjustment_avg_ns;		/*clock adjustment (nanoseconds) average or [nan] if not determined*/

			//requires X amount of data/samples before returning data.
			bool has_stats() const { return !(std::isnan(network_latency_jitter_ns) || std::isnan(clock_adjustment_jitter_ns)); }

			void clear(){
				clock_adjustment_jitter_ns = network_latency_jitter_ns =
				network_latency_avg_ns = clock_adjustment_avg_ns =
				std::numeric_limits<double>::signaling_NaN();
				nb_samples = 0;
			}

			DescriptiveStatistics() { clear(); }
		}ds;
	}; 
	typedef std::shared_ptr<ntp::Metrics> MetricsPtr;
	
	enum class Status : int16_t
	{
		Ok = 0,
		UnknownErr = 1,
		InitSockErr = 2,
		CreateSocketErr = 3,
		SendMsgErr = 4,
		ReceiveMsgErr = 5,
		ReceiveMsgTimeout = 6,
		SetOsTimeErr = 7,
		AdminRightsNeeded = 8,
		TimeAdjustLimitMax = 9,
		TimeAdjustLimitMin = 10,
		LatencyLimitMax = 11,
		ServerOutOfSynch = 12,
	};

	class Client;//forward declaration
	

	class Result
	{
		friend class ntp::Client;
		Status m_status;
		//the host name/ip of the ntp server
		ntp::EndPointPtr m_server_ep;
		//actual time value that was extracted
		ntp::time_point_t m_time;
		//actual time that the time value was extracted.
		std::chrono::steady_clock::time_point m_extraction_time;
		
		/* metrics values with other relevant data*/
		Metrics m_mtr;			  
		//when set, [m_used_for_setting_clock] indicated that the results was used to update the internal clock of the client.
		bool m_used_for_setting_clock;	
		
		//When exists,holds details on the server that responded(Available only after successfull requests).
		ServerInfoPtr m_server_ext_info;

	public:
		Result();

		inline operator bool() const { return Status::Ok == m_status; }
		
		//Status can differ from Status::OK, while the time query can be valid.
		//For instance, if error toke place when trying to set the system clock
		//or the status is set to "TimeAdjustLimitMin", i.e. local time adjustment did not took place.
		inline bool has_time() const { return ntp::time_point_t::min() != m_time; }

		//Indicated that the results was used to update the internal clock of the client
		bool used_for_setting_clock() const{ return m_used_for_setting_clock; }

		std::string as_string() const;
		
		inline ntp::Status status() const { return m_status; }

		//Returns the network and clock related metrics at the time of the request was made.
		inline const ntp::Metrics& mtr() const { return m_mtr; }

		//Returns the endpoint of the NTP server
		inline const EndPointPtr server() const { return m_server_ep; }

		//Returns the server info that was available for this request.
		//Only available for request, for which the server responsed to (status > Status.SetTimeErr);
		inline const ServerInfoPtr server_info() const { return m_server_ext_info; }

		//Returns the time when the time value wa set.
		//will return std::chrono::steady_clock::time_point::min() if [has_time()] is false;
		inline std::chrono::steady_clock::time_point time_extraction_time() const { return m_extraction_time; }
		
		//Returns the time value time extracted from the NTP server.
		//will return empty time object if [has_time()] is false;
		inline ntp::DateTime time() const { return ntp::DateTime(m_time); }

		//Returns the current time based on the time extracted.
		//The returned value will be different on each call.
		//will return empty time object if [has_time()] is false;
		inline ntp::DateTime now() const;

	};	
	typedef std::shared_ptr<ntp::Result> ResultPtr;

	enum class LocalTimeSource {
		//SystemTime: Using the system clock as time reference whenever, comparing agains the server. 
		//			  When using this method without synching the system clock, the deltas & offsets reported will not be reduced.
		SystemTime,
		//InternalClock: Using the system clock as time reference - only for thr 1st time, when comparing agains the server. 
		//			   On he following calls the client's internal clock is used as reference and for updated.
		//			   When using this method the client is not sensitive to changes on the system clock nor it it effecting it.
		InternalClock
	};

	enum class FineTuneOverReason {
		None,					//Fine tune is not over.
		FinetuneDisabled,	    //Fine tuning proecdure is disabled (0 > settings.fta.fine_adjustment_duration.count())
		EstablishingComTimedout,//Connection to server was not established after [ClientSettings::ClockAdjustmentRules::max_com_establish_duration].
		FineTuningTimedout,		//The maximal duration allocated for fine tunning has timedout (ClientSettings::fta.fine_adjustment_duration)
		QueriesMaxedOut,		//Amount of server queries during fine tube procedure maxed out [ClientSettings::fta.nb_queries].
		ReachedTargetClockAdjustment,//Ahe amount of clock adjusemets eached <= settings.fta.delta_sd_min [ClientSettings::fta.nb_queries].
	};

	class IClient;
	typedef std::shared_ptr<ntp::IClient> ClientPtr;

	struct ClientSettings
	{
		//This enum specefy the protocol used to negotiating with the server.
		enum class RequestedProtocolVersion : uint8_t {
			//Auto select basted on the network and server's capatability.
			AutoSelect = 0,
			//Version 3 (IPv4).
			V3 = 3,
			//Version 4 (IPv4, IPv6 and OSI).
			V4 = 4,
		};

		//specefies the requested ntp version to be used.
		RequestedProtocolVersion version = RequestedProtocolVersion::AutoSelect;

		//When set, drift calculation will be used to estimate the inner clock drift and correct them.
		bool mitigate_clock_drift = true;

		//When set to non-empty string drift table will be stored, and (if exist load) on the fiven file name
		std::string drift_table_backup_file_name;

		//When set (very recommanded), the calced clock adjustemt will be smoothed out for smoother transition from client clock and server's.
		//In addition it will help mitigate jitters in clock updates.
		bool smooth_clock_adjustmet = true;

		//Local time source (see LocalTimeSource)
		LocalTimeSource lts = LocalTimeSource::InternalClock;

		struct ClockAdjustmentRules 
		{
			//Maximal (abs) clock adjusting value. Time delta of a greater duration will be ignored
			//and query result will carry the [TimeAdjustLimitMax] error.
			//note: see [ignore_max_clock_adjustment_on_start]
			std::chrono::nanoseconds max_clock_adjustment = std::chrono::minutes(2);

			//When set, [max_clock_adjustment] limitation will be ignored while IClient.has_time() returns false.
			//This is critical on the 1st stages of synchronization, especialy if the local system clock is far from the "real time".
			bool ignore_max_clock_adjustment_on_start = true;

			//Minimal (abs) time adjusting value. Time delta of a lower duration will be ignored
			//and query result will carry the [TimeAdjustLimitMin] status.
			//Using this value can contribute for the stability/consistency of the internal clock,
			//by perventing over adjustment.
			//Note, this limitaition will be ignored:
			//1. During the fine tuning procedure (See FineTuneAdjustment settings).
			//2: When the network jitter will be lower or equal to [MinimalClockAdjustmentRules::max_network_jitter]
			std::chrono::nanoseconds min_clock_adjustment = std::chrono::seconds(30);

			//Maximal (abs) network latency, for time adjusting.
			//Server's response with heigher delay will not be used for time adjustment.
			//and query result will carry the [TimeAdjustLimitMax] status.
			//Using this value can contribute for the stability/consistency of the used clock,
			//perventing over adjustment of the clock, setting the value to 0 or negative will disable the option.
			std::chrono::nanoseconds max_latency = std::chrono::nanoseconds(0);

			//defines the maximal time the client will wait for server response when performing query.
			//setting this to too low will lead to lots of query failues with Status::RECEIVE_MSG_TIMEOUT.
			//set to 0 to use default timeout.
			std::chrono::milliseconds server_response_timeout = std::chrono::milliseconds(0);

		}clock_adjustment_rules;

		//Conditions for inner clock update, that will overule the ClientSettings::ClockAdjustmentRules::min_clock_adjustment settings.
		//The idea is to keep the inner clock as updated as possible even if it did not qualfied for the minimal delta from the server.
		//-This is done if the network behaviour is stable and/or the deltas mesured from the server are stable(this implicitly holds network stability component).
		//-Note that on some systems, large intervals between query operation may lead to unstable network behaviour.
		struct MinimalClockAdjustmentRules
		{
			//When the network jitter will be lower or equal to [max_network_jitter],
			//Time adjustment will take place, even if the TimeAdjustLimitMin will be assigned.
			//This will allow updating the internal clock, in cases we trust the network stability.
			//Jitter under 1 millisecond is normally consideted stable.
			//To disable this just set it to std::chrono::microseconds::min().
			std::chrono::microseconds max_network_jitter = std::chrono::microseconds(750);

			//When the required time adjustements jitter will be lower or equal to [max_clock_adjustment_jitter],
			//Time adjustment will take place, even if the TimeAdjustLimitMin will be assigned.
			//This will allow updating the internal clock, in cases we trust the clocks delta stability (its includes network stability component).
			//Jitter under 1 millisecond is normally consideted stable.
			//To disable this just set it to std::chrono::microseconds::min().
			std::chrono::microseconds max_clock_adjustment_jitter = std::chrono::microseconds(750);

		}minimal_clock_adjustment_rules;

		//This struct describes the fine tune clock adjustment feature and the operating conditions.
		//Fine tune adjustment is procedure that the client will try and fine tune the internal clock and improve it's accuracy.
		//When the procedure is active, if reliable oportunity to improve the accuracy comes across, the [ClientSettings::ClockAdjustmentRules::min_clock_adjustment] is ignored.
		//This procedure starts after the 1st successfull time query of a server,
		//and will start again if the ntp server is replaced with another.
		//see is_fine_tunning() and is_fine_tunning_over() methods.
		//The fine tunning procedure will stop on the 1st stoping condition that will meet the critirias.
		struct FineTuneAdjustment
		{
			//Defines maximal duration from first the query time to a server (successful or not),
			//till the time actual time info will be returned from the server.
			//If by this duration no time info will bre returned, [is_fine_tunning_over] will return true even though no time info exist.
			std::chrono::microseconds max_com_establish_duration = std::chrono::seconds(10);

			//Defines a time duration after the 1st successful time query of a server, that the fine tune procedure will operate.
			//set to zero to allow infinite fine tunning or negative value to disable the feature.
			std::chrono::microseconds fine_adjustment_duration = std::chrono::seconds(60);

			//When set to value > 0 will cause the fine tune procedure to stop if the jitter of the
			//delta values between the client's clock and the server clock drops below that value.
			//this is harder to achive on less stable networks (such as wifi) 
			std::chrono::microseconds clock_adjustment_jitter_min = std::chrono::milliseconds(1);

			//When set to none zero value, fine adjustment will stop after [nb_queries] sucessfull queries.
			uint16_t nb_queries = 0;
		} fta;

		typedef void(*OnError)(const ntp::IClient& client, std::string error_string, void* userParam);
		typedef void(*OnWarning)(const ntp::IClient& client, std::string warning_string, void* userParam);
		typedef void(*OnClockSet)(const ntp::IClient& client, ManagedClockPtr new_clock, void* userParam);
		OnError			error_callback = nullptr;
		OnWarning		warnning_callback = nullptr;
		OnClockSet		clockset_callback = nullptr;
		void* userParam;
	};	
	

	class IClient
	{
	public:

		static ClientPtr create_instance();

		virtual ~IClient() {};
		IClient() {};

		//Init the client (should be done only once) 
		virtual void init(const ntp::ClientSettings& settings) = 0;

		//Attempts to query the ntp server
		//invalidate_clock: when set, the existing inner clock  will be reset (on this call),
		//					clear/reset all statisics, history, clocks and time info
		//					and start again. Doing that will revoke the time info and 
		//ntpServerEp:		The endpoint to connect to. On the 1st call the ntpServerEp must not be empty.
		//					In case the next calls are with empty end points, it will maintain the existing connection setup.
		//					In case the given endpoint is not empty and differ from the curretly "connected" endpoint,
		//					the inner ntp server will be replaced and the inner clock and stats will be revoked.
		virtual ResultPtr query(ntp::EndPointPtr ntpServerEp, bool invalidate_clock) = 0;
	

		//Attempts to manuly set the local system clock given the query result.
		//this method is not used be the clinet and should be used by teh client's discretion.
		//Note that the given ntp query result must have time value on it.
		virtual bool set_system_clock(const ResultPtr res) const= 0;

		//Reutrns whether or not the clinet has/gained a clock that can be used to extract time.
		//this will be false if no query result returned with time information, after the last server selection.
		virtual bool has_time() const = 0;

		virtual bool is_fine_tunning() const = 0;
		virtual bool is_fine_tunning_over() const = 0;

		//Returns the last clock that was sycnhed/updated by the client.
		//The managed clock can be used seperatly to get "current" time at any given moment (without counting for the internal machine clock drift).
		//If the has_time() call returns false this function will return nullptr.
		virtual ntp::ManagedClockPtr clock() const = 0;

		//returns the reason for finetune procedure to be over.
		virtual FineTuneOverReason finetune_over_reason() const = 0;

		//Returns the result of the last query that generated internal clock update.
		//Normally this will be the 1st successfull query and after that fine tuning procedure result or 
		//results with clock ClientSettings::ClockAdjustmentRules::min_clock_adjustment < delta < max_clock_adjustment.
		//In other words results that calling res.used_for_setting_clock() on them will return true.
		//It can also be null if no clock updates took place since teh last server selection.
		virtual ResultPtr last_clock_update_query_result() const = 0;

		//Returns the endpoint info of the last server that was queries (or null of no query attempt was yet to be made).
		//as apposed to [server_info], this will be available without sucessfull query results.
		virtual EndPointPtr queried_server_endpoint() const = 0;

		//Returns the server info only available after the 1st sucessfull request,
		//for which the server responsed to(status > Status.SetTimeErr)
		virtual const ServerInfoPtr server_info() const = 0;	

		//Returns the most updated network and clock related metrics;
		virtual ntp::MetricsPtr mtr() const =0;

		//Returns 1.0 - [SERVER_MSG_TRANSMISSION_ERRORS_COUNT]/[TOTAL_QUERIES]. i.e 0.0 <= value <= 1.0 .
		//if no query was performed will return 0.
		virtual double server_queries_success_rate() const = 0;

		//returns the duration from the last time the internal clock weas updated.
		//This wil return negarive value if [has_time] returns false (clock was not set).
		virtual std::chrono::nanoseconds duration_from_last_clock_update() const = 0;

		//returns the duration from the query that returned time info from the server.
		//This wil return negarive value if [has_time] returns false (clock was not set).
		virtual std::chrono::nanoseconds duration_from_successful_query() const = 0;

	private:
		IClient(const IClient&);			  //disabled
		IClient& operator = (const IClient&); //disabled
	};



	class ntp_client_kickoff_intervals_algorithm {
		std::chrono::steady_clock::time_point kickoff_started = std::chrono::steady_clock::time_point::min();
	public:
		std::chrono::milliseconds sleep(ntp::ClientPtr client) {

			if (std::chrono::steady_clock::time_point::min() == kickoff_started)
				kickoff_started = std::chrono::steady_clock::now();

			if (!client->server_info())//cont connected to any server yet
				return std::chrono::seconds(20);

			//Too large query intervals will likely cause higher network jitter effect
			std::chrono::milliseconds post_kick_off_min_intervals(std::chrono::seconds(5));
			if (post_kick_off_min_intervals < client->server_info()->min_query_intervals)
				post_kick_off_min_intervals = client->server_info()->min_query_intervals;

			if (client->is_fine_tunning_over())	
			{
				auto intervals = post_kick_off_min_intervals;

				if (client->has_time())	intervals = post_kick_off_min_intervals;

				std::this_thread::sleep_for(intervals);
				return intervals;
			}			

			//to prevent entering NTP server's blacklist:
			//start qurey at :			 3 per sec
			//after 6 seconds drop to :  2 per sec
			//after 10 seconds drop to : 1 per sec
			//after 15 seconds drop to : 1 per sec
			//after that seconds drop to : max of 15/1 per second
			double queries_per_second = 0;
			const auto secs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - kickoff_started).count() / 1000.0;
			if (secs < 6)
				queries_per_second = 3;
			else if (secs < 10)
				queries_per_second = 2;
			else if (secs < 15)
				queries_per_second = 1;
			else
				queries_per_second = 15/ secs;
		
			std::chrono::milliseconds intervals(static_cast<int64_t>(1000.0/queries_per_second + 0.5));

			//reduce the intervals if possible:
			if (client->has_time() && intervals > client->server_info()->min_query_intervals)
				intervals = client->server_info()->min_query_intervals;
			else {
				if (intervals > post_kick_off_min_intervals)
					intervals = post_kick_off_min_intervals;
			}

			std::this_thread::sleep_for(intervals);// sleep for specified time
			return intervals;
		}
	};
}
#endif//__intp_client_h__