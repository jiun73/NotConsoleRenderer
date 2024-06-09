#ifndef __ntp_client_h__
#define __ntp_client_h__

#include <chrono>
#include <map>
#include <string>
#include <memory>
#include <thread>
#include <shared_mutex>
#include <list>
#include "include/intp_client.h"
#include "ntp.h"
#include "utils.h"
#include "manual_event.h"

namespace ntp
{
	class Client : public ntp::IClient
	{

	public:

		Client();
		virtual ~Client();

		//Init the client
		virtual void init(const ClientSettings& settings) override;

		virtual ResultPtr query(ntp::EndPointPtr ntpServerEp, bool invalidate_clock) override;

		virtual bool set_system_clock(const ResultPtr res) const override;

		virtual bool has_time() const override;

		virtual bool is_fine_tunning() const override;

		virtual bool is_fine_tunning_over() const override;

		virtual FineTuneOverReason finetune_over_reason() const override;

		virtual ManagedClockPtr clock() const override;

		virtual const ServerInfoPtr server_info() const override;

		virtual ntp::MetricsPtr mtr() const override;

		virtual ResultPtr last_clock_update_query_result() const override;

		virtual EndPointPtr queried_server_endpoint() const override;

		virtual double server_queries_success_rate() const override;

		virtual std::chrono::nanoseconds duration_from_last_clock_update() const override;

		virtual std::chrono::nanoseconds duration_from_successful_query() const override;

	private:		
		
		Client(const Client&);				//disabled
		Client& operator = (const Client&);	//disabled

		//Max mount of samples required to calculate staistics.
		//note that the fine tuning procedure relays on this stats and requiring wider window will prolong the time required 
		//to start the procedure.
		static const uint16_t STATS_CALC_WINDOW_SIZE;

		inline bool has_time_private() const{//Non virtual version of "has_time()" to make it inline.
			return
				time_point_t::min() != m_ntp_server_state.m_time &&
				std::chrono::steady_clock::time_point::min() != m_ntp_server_state.m_prev_clock_update_time;
		}		
				
		inline ntp::time_point_t now_private(bool supresss_drift = false) const{
			std::chrono::nanoseconds ref_clock_dur = (std::chrono::steady_clock::now() - m_ntp_server_state.m_prev_clock_update_time);
			if (supresss_drift && 0 != m_ntp_server_state.m_drifts.m_drift_rate)
				ref_clock_dur = ref_clock_dur + ntp::utils::steady_clock_accumulated_drift(m_ntp_server_state.m_drifts.m_drift_rate, ref_clock_dur);
		
			return (m_ntp_server_state.m_time + ref_clock_dur);
		}

		inline ntp::time_point_t now_private(bool supresss_drift, std::chrono::nanoseconds& out_drift_component ) const {
			std::chrono::nanoseconds ref_clock_dur = (std::chrono::steady_clock::now() - m_ntp_server_state.m_prev_clock_update_time);
			out_drift_component = ntp::utils::steady_clock_accumulated_drift(m_ntp_server_state.m_drifts.m_drift_rate, ref_clock_dur);
			if (supresss_drift)
				ref_clock_dur = ref_clock_dur + out_drift_component;

			return (m_ntp_server_state.m_time + ref_clock_dur);
		}
				

		ntp::Status init(ntp::EndPointPtr ntpServerEp);

		void cleanup_socket_and_address();

		//attempts to quey the server and update members on teyh result.
		//returns true if time values where set on the result.
		bool query_private(ResultPtr in_out_result);

		//performs query operaiton on seperated thread.
		void asynch_query_work();

		//Create, writes and parses drifts table.
		void manage_drifts_table(const ResultPtr query_result);

		ntp::ClientSettings m_settings;
		void* m_p_si_addr;
		int m_p_si_addr_size;
		int m_address_family;
		size_t m_socket;

		NtpPacketBuffer m_out_ntp_msg, m_in_ntp_msg;
		
		struct ServerState 
		{
			EndPointPtr m_ep;				//host name/or address string that the client resolving to connected the to server.
			IPAddressPtr m_ep_ip;			//resolved address that the client is connecting to the server with.
			ProtocolVersion m_protocol_ver;	//The version of ntp protocol that the client is/should be using with the server.
			double m_offset_ns;				//clock offset from server + network delay.
			double m_delay_ns;				//network delay (one way)
			double m_delta_with_drift_ns;	//delta from server (offset - delay)
			double m_delta_no_drift_ns;		//delta from server (offset - delay - drift_component)
			double m_drift_ns;				//accumulated drift since last clock update

			Metrics::DescriptiveStatistics m_sd;//statistical data.

			time_point_t m_time;
			//[m_prev_clock_update_time] can be used to synchronize inner timer without relaying on system's timer updates.
			std::chrono::steady_clock::time_point m_prev_clock_update_time;

			//holds the last results used for clock update.
			ResultPtr m_clock_update_result;
			//metrix of the last sucessfull query result (not necessarily used fot clock update)
			ntp::MetricsPtr m_last_result_mtr;
			std::chrono::steady_clock::time_point m_fine_tune_procedure_start_time;
			mutable bool m_fine_tune_procedure_over;

			std::chrono::steady_clock::time_point m_server_com_init_start_time;
			struct drifts_data{
				//drift table stores client_time and the delta_ns calculated for that time.
				std::list<std::pair<ntp::time_point_t, double>> m_drift_table;
				//The last calcualted drift rate, expressed in drift of T the internal clock duration.
				//Positive: local clock is slower, negative: local clock is faster.
				//For example value of 0.01, suggests that the clock drifts at 1% per unit of time.
				//in order to mitigate the clock's drift: fixed_duration = duration + (drift_rate*duration)
				//Besure NOT to do fixed_duration = duration * (drift_rate+1.0); as drift_rate can be negative
				double m_drift_rate;
				std::chrono::steady_clock::time_point last_time_calced;
				std::chrono::steady_clock::time_point last_time_backup;
				std::chrono::seconds time_from_last_calc() const;
				std::chrono::seconds time_from_last_backup() const;
				void clear();
			}m_drifts;

			void clear();
			ServerState();
			~ServerState() { clear(); }
		}m_ntp_server_state;

		struct stats{
			void clear();
			bool has_queries_success_rate() const { return !std::isnan(m_queries_success_rate); };
			double m_queries_success_rate;
			uint64_t m_nb_server_calls;//counts the nb of server calls though the the entire live of the server connection/
			uint64_t m_nb_stats_server_calls;//counts the nb of server calls, but get cleard from time to time.
			uint64_t m_nb_server_msg_transmission_errors;
			FineTuneOverReason m_finetune_over;
			std::chrono::steady_clock::time_point m_prev_sucessful_query_time;
			std::list<double> m_network_latency;
			std::list<double> m_clock_deltas;
		}
		mutable m_stats;

		//when set, the clock that will be used to synchronized agains will be the system's clock
		//otherwise, the system clock will only be used for initial value and from that point on, clock used for synching
		//with remote server will be internal and independent.
		bool m_should_use_system_clock;
		mutable std::shared_mutex m_clock_lock;
		mutable std::shared_mutex m_mtr_lock;

		//The server's query are all done on the same thread to be able to control its priority
		//and potentialy reduce the variation/drift quartz crystal oscillators used by the machine.
		std::thread m_asynch_query_thread;
		
		struct query_job
		{	
			ResultPtr in_out_query_result;
			bool query_sucess;
			manual_event query_thread_job_started_evt;
			manual_event query_thread_job_over_evt;
			query_job() : query_sucess(false), query_thread_job_started_evt(false), query_thread_job_over_evt(false){}
			bool push_wait_query(ResultPtr _in_out_query) {
				in_out_query_result = _in_out_query;
				query_thread_job_over_evt.reset();
				query_thread_job_started_evt.set();
				query_thread_job_over_evt.wait();
				return query_sucess;
			}
		}m_qj;
		
	};

	
}
#endif//__ntp_client_h__