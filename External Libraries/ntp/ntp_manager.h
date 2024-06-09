#ifndef __ntp_mng_h__
#define __ntp_mng_h__

#include "include/intp_manager.h"
#include <thread>
#include <shared_mutex>
#include "manual_event.h"

namespace ntp
{
	class Manager : public ntp::IManager
	{
	public:

		Manager();
		virtual ~Manager();

		//Init the client (should be done only once) 
		virtual void begin_init(const ManagerSettings& settings) override;

		virtual State wait_initialzed(std::chrono::microseconds waitDuration) const override;

		virtual State state(std::chrono::microseconds waitDuration) const override;

		virtual std::pair<ntp::ServerInfoPtr, ServerRating> active_server() const override;

		virtual bool has_time() const override;

		virtual ManagedClockPtr clock() const override;

		virtual bool is_clock_monotonous() const override;

		virtual void set_clock_as_monotonous(bool enable_monotonous) override;

	private:
		Manager(const Manager&);				//disabled
		Manager& operator = (const Manager&);	//disabled

		void manager_work();

		void update_state(const State& st);
		void update_clock(ntp::ManagedClockPtr new_clock);
		ntp::ClientPtr select_best_server(ServerRating &outRating) const;
		ntp::ClientPtr create_client(EndPointPtr ep) const;
		void init_server_com(ntp::ClientPtr client, ManagerSettings::ServersEvaluationMethod mthd) const;

		mutable std::shared_mutex m_server_lock;
		mutable std::shared_mutex m_time_lock;
		mutable std::mutex m_state_lock;
		State m_state;
		ServerRating   m_activeServerRating;
		ntp::ClientPtr m_activeServer;
		ntp::ManagedClockPtr m_clock;
		std::thread m_worker;
		manual_event m_disposedEvt;
		manual_event m_serverSelectedEvt;
		manual_event m_force_oneshort_clock_updateEvt;
		ManagerSettings m_settings;	
		//When the manager's is set as having monotonous clock, it will not perform clock update except 
		//for the one done after a server is selected, it will also ignore the shelf life experation if such took place.
		bool m_is_monotonous_clock;

	};
}

#endif// __ntp_mng_h__