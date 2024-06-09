#ifndef __intp_server_h__
#define __intp_server_h__

#include <string>
#include <memory>
#include "intp.h"
#include "intp_manager.h"

namespace ntp
{
	class IServer;
	typedef std::shared_ptr<ntp::IServer> ServerPtr;

	class IServer
	{
	public:

		struct ServerSettings
		{
			//When set, binding to the address and port will prohibit from other listeners to bind to the adress and port.
			bool exclusive_address_usage = true;

			//When set, the server will report to its clients that it's clock's root_dispersion and root_delay
			//are the both the minimum possibe, reguardless of the actual source server's clock (reference clock) accuracy/stability.
			//Using it may be usefull for "persuading" client's that have minimal requirements, to synch with the server even in such circumstances.
			//*Note: When source server's clock (reference clock) is not configured, the ntp::IServer will report zero dispersion whether or not this flag is set.
			bool report_minimal_root_dispersion_delay = false;

			//When set the server will use this NTP manager as time source for it's clients.
			//when not set, the system time on initialization will bne used as a reference clock.
			ClientsListManagerPtr time_source;			

			//Invoked whever a client is successfully served with time information
			typedef void(*OnClientServed)(const ntp::IServer& server, std::string client_addr, void* userParam);

			typedef void(*OnError)(const ntp::IServer& server, std::string error_string, void* userParam);
			typedef void(*OnWarning)(const ntp::IServer& server, std::string warning_string, void* userParam);

			OnClientServed client_served_callback = nullptr;
			OnError			error_callback = nullptr;
			OnWarning		warnning_callback = nullptr;
			void* userParam;
		};

		static ServerPtr create_instance();

		virtual ~IServer(){};
		IServer() {};

		//Init the server
		virtual bool init(const ntp::IServer::ServerSettings& settings, const ntp::EndPointUDP& listen_ep = ntp::EndPointUDP(ntp::SpecialAddress::ADDR_ANY_IPV4, ntp::UDP_PORT)) = 0;

		//returns the clock used by the server;
		virtual ManagedClockPtr inner_clock() const = 0;

		//turns out of synch flag on or off (defaults to off).
		virtual void flag_as_out_of_synch(bool out_of_synch) =0;

	private:

		IServer(const IServer&);				//disabled
		IServer& operator = (const IServer&);	//disabled
	};
}

#endif//__intp_server_h__
