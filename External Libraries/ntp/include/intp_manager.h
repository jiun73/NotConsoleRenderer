#ifndef __intp_mng_h__
#define __intp_mng_h__

#include "intp_client.h"
#include <chrono>
#include <string>
#include <memory>
#include <stdint.h>
#include <list>

namespace ntp
{
	class IManager;
	typedef std::shared_ptr<ntp::IManager> ClientsListManagerPtr;

	//Interface for a ntp clients manager.
	//Its select's the best ntp server out of X servers and potentialy renewing/demoting a server.

	class IManager
	{
	public:

		static ClientsListManagerPtr create_instance();

		enum class ServerRating {//rates ntp server by network latency, stability, availabiliy etc.. 
			Superb,
			Excellent,
			Good,
			Satisfactory,
			Poor,
			Invalid,//No active server
		};

		enum class StatusCode {
			InvalidSettings,//One or more settings parameters ar invalid (can take place only during initilaiztion procedure).
			Initializing,//Init is still running (can take place only during initilaiztion procedure).
			NewServerObtained,//Best server was selected and synchronized.
			ClockUpdated,//The clock used just got updated.
			ServerAvailablitityDropped,//The active sever's availablitity drroped under settings.asd.min_availablitity.
			ClockShelfLifeExpired,//The shelf life of the current clock has expired.
			NoServer,//Failed selecting a server (Initialization failed)
			Unkown,//Initialization was not invoked(default state).
		};

		class State {
			StatusCode m_code;
			std::string m_msg;
		public:
			operator bool() const { 
				return StatusCode::InvalidSettings != m_code && StatusCode::NoServer != m_code && StatusCode::Unkown != m_code;
			}
			operator std::string() const { return "State: " + m_msg; }
			operator StatusCode() const { return m_code; }
			std::string msg() const { return "State: " + m_msg; }
			StatusCode code()const { return m_code; }
			State(StatusCode _code = StatusCode::Unkown, std::string _msg = std::string("Unkown")):
				m_code(_code), m_msg(_msg){}
		};		

		struct ManagerSettings 
		{
			//Describes the duration from the last successfull time query, for which the internal clock will be be considered 'out-of-date'.
			//once the inner clock is considered out of date, and multiple severs endpoints are configured there will be attempt to evaluate the best server
			//among them and set it as the active server.
			//[has_time()] call will return false.
			//setting the value to zero, will make the clock's shelf life infinite.
			std::chrono::microseconds clock_shelf_life = std::chrono::seconds(0);
	
			//The minimal time intervals betwen remote ntp server query operations.
			//i.e, query operation will never be more rappid then this value.
			//Notes:
			//-The actual frequency can be lower if the server supports only lower quey frequency.
			//-Too large query intervals will likely cause higher network jitter.
			std::chrono::seconds min_server_query_frequency=std::chrono::seconds(4);

			//AutoServerDemoting as an optional mechanizem to demote under cirtain conditions
			struct AutoServerDemoting {
				//0.0=< min_availablitity <=1.0  when set to 0 will not be a trigger for demoting a server.
				//Trying to reevalute the servers if the current one's drops under this percentage value.
				double min_availablitity = 0.6;

			}asd;	

			ClientSettings ncp_client_settings;

			//Collection of ntp servers endpoints to attempt connect to.
			std::list<EndPointPtr> ntp_servers;

			//Describes  the way thhat servers will be evaluated.
			enum class ServersEvaluationMethod{
				Comprehensive,//This method is likely to take longer time and uses more complex matrix to evaluate the servers.
				Fast,//This method will use reduced mesuements steps, evaluating the best server faster.
				UltafastFast,//This method will use the minimal tests to evaluate the best server, which will make it faster.
			} servers_eval_method = ServersEvaluationMethod::Comprehensive;

			//When [start_with_system_clock] is set, the manager will be nitialized at the begining with system clock.
			//that means that "has_time" will be true even if time from ntp servers was not estabvlished yet.
			bool start_with_system_clock = true;

			//When set, the system clock will update whenever the internal clock is changed.
			bool update_system_clock_on_clock_change = false;

			typedef void (*OnStateUpdated)(const ntp::IManager& mng, ntp::IManager::State newState, void* userParam);
			typedef void(*OnServerEvaluated)(const ntp::IManager& mng, ntp::EndPointPtr server, ntp::IManager::ServerRating rating, double grade, const ntp::Metrics & rating_metrics, std::string as_string, void* userParam);
			typedef void(*OnServerRequestResult)(const ntp::IManager& mng, ntp::ResultPtr ntp_request_result, void* userParam);

			OnServerEvaluated server_evaluated_callback = nullptr;
			OnStateUpdated state_callback = nullptr;
			OnServerRequestResult request_result_callback = nullptr;
			void* userParam = nullptr;
		};



		virtual ~IManager() {};
		IManager() {};

		//Starts the manager's asynchonius initialzation.
		//In this procedure the manager will try conenct and evaluate and rate the servers at the end points,
		//from which the 'best' server will be chosen.
		//One should call "wait_initialzed" right after to get better understanding of the init state/result.
		virtual void begin_init(const ntp::IManager::ManagerSettings& settings) = 0;

		//Wait untill a server will be selected
		//waitDuration: maximal duration to wait or negative for infinite.
		virtual State wait_initialzed(std::chrono::microseconds waitDuration) const= 0;

		//Returns the current state of the manager.
		//After [wait_initialzed] returns (end of initializetion) 
		//The state may change, for instance if [clock_shelf_life]  is set and servers failes to connect.
		virtual State state(std::chrono::microseconds waitDuration) const = 0;

		//Returns info on the of the active server or empty one(set to nullptr).
		virtual std::pair<ServerInfoPtr, ServerRating>  active_server() const = 0;

		virtual bool has_time() const = 0;

		//Returns time information. will return object that will not carry time if [has_time] returns false,
		virtual ntp::ManagedClockPtr clock() const = 0;

		//Indicates whether the manager's clock is configured as monotonous. 
		//when set as such, it will not perform clock update except for the one done after a server is selected.
		//It will also ignore the shelf life experation if such took place.
		virtual bool is_clock_monotonous() const = 0;

		//Sets whether the manager's clock is configured as monotonous. 
		//when set as such, it will not perform clock update except for the one done after a server is selected.
		//It will also ignore the shelf life experation if such took place.
		virtual void set_clock_as_monotonous(bool enable_monotonous) = 0;

	private:
		IManager(const IManager&);				//disabled
		IManager& operator = (const IManager&); //disabled
	};
	
}//end of namespace

#endif// __intp_mng_h__