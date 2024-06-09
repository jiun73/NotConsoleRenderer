#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <thread>

#include "tests.h"

#include "../../ntp/include/intp_client.h"
#include "../../ntp/include/intp_manager.h"
#include "../../ntp/include/intp_server.h"

std::list<ntp::EndPointPtr> compile_ntp_endpoints(bool include_local_ntp)
{
	std::list<ntp::EndPointPtr> ntp_servers;
	if (include_local_ntp)
		ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>(ntp::SpecialAddress::LOOPBACK_IPV4));

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.google.com"));//Google: poll intervals of 1
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time1.google.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time2.google.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time3.google.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time4.google.com"));

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.cloudflare.com"));// Cloudflare: poll intervals of 1

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.facebook.com"));//Facebook : poll intervals of 1
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time1.facebook.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time2.facebook.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time3.facebook.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time4.facebook.com"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time5.facebook.com"));

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.windows.com"));//Microsoft: poll intervals of 1

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.apple.com"));//Apple: poll intervals of 8

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("tik.cesnet.cz"));//poll intervals of 8

#if 0
	//very low relability:
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("0.ubuntu.pool.ntp.org"));//Ubuntu community: poll intervals of 1
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("1.ubuntu.pool.ntp.org"));

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("0.nettime.pool.ntp.org"));//Netime project: poll intervals of 1
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("1.nettime.pool.ntp.org"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("2.nettime.pool.ntp.org"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("3.nettime.pool.ntp.org"));

	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("pool.ntp.org"));//poll intervals of 1
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("0.pool.ntp.org"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("1.pool.ntp.org"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("2.pool.ntp.org"));
	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("3.pool.ntp.org"));

#endif
	return ntp_servers;
}

void execute_ntp_client_query_loop(ntp::EndPointPtr ep)
{
	std::ofstream log_file(std::string("c:\\temp\\") + ep->host + ".log", std::ios::trunc);

	ntp::ClientSettings cs;

	//Conditions for inner clock update, that will overule the ClientSettings::ClockAdjustmentRules::min_clock_adjustment settings.
	//The idea is to keep the inner clock as updated as possible even if it did not qualfied for the minimal delta from the server.
	//-This is done if the network behaviour is stable and/or the deltas mesured from the server are stable(this implicitly holds network stability component).
	//-Note that on some systems, large intervals between query operation may lead to unstable network behaviour.
	cs.minimal_clock_adjustment_rules.max_network_jitter = std::chrono::microseconds(750);
	cs.minimal_clock_adjustment_rules.max_clock_adjustment_jitter = std::chrono::microseconds(750);

	//Do not alter the machines clock, use internal clock instead:
	cs.lts = ntp::LocalTimeSource::InternalClock;

	//Setting the fine-tuning procedure stopping conditions:
	cs.fta.clock_adjustment_jitter_min = std::chrono::microseconds(750); //Demanding, but possible.
	cs.fta.fine_adjustment_duration = std::chrono::seconds(60);

	cs.error_callback = [](const ntp::IClient& client, std::string error_string, void* userParam) {
		if(client.queried_server_endpoint())
			std::cout << "NTP client error: " << client.queried_server_endpoint()->as_string() << ' ' << error_string << std::endl;
		else
			std::cout << "NTP client error: "<< error_string << std::endl;
	};

	cs.warnning_callback = [](const ntp::IClient& client, std::string warn_string, void* userParam) {
		std::cout << "NTP client warning: " << client.queried_server_endpoint()->as_string() << ' ' << warn_string << std::endl;
	};

	cs.clockset_callback = [](const ntp::IClient& client, ntp::ManagedClockPtr new_clock, void* userParam) {
		std::cout << "NTP client clockset: " << client.queried_server_endpoint()->as_string() << ' ' << new_clock->now().as_local_time_string() << std::endl;
	};

	auto ntp_client = ntp::IClient::create_instance();
	ntp_client->init(cs);

	bool synched = false, acquired_time = false;
	ntp::ntp_client_kickoff_intervals_algorithm kia;
	auto queryProcedure = [&synched, &acquired_time, &ntp_client, &log_file, &ep, &kia]()
	{
		auto res =														 // main NTP server query
			ntp_client->query(ep, false);

		const bool ft_over = ntp_client->is_fine_tunning_over();
		std::cout << res->as_string() << std::endl;// print status string
		std::cout << "*********************************************************************************************************************" << std::endl;
		log_file << res->as_string() << std::endl;
		log_file  << "*********************************************************************************************************************" << std::endl;

		if (synched != ft_over)
		{
			synched = ft_over;
			std::stringstream ss;
			ss << "\t**********************************************************************************" << std::endl
				<< "\t\t " << (synched ? "Synched" : "Out of synch") << " Reason code: " << (int)ntp_client->finetune_over_reason() << std::endl
				<< "\t**********************************************************************************" << std::endl;

			std::cout << ss.str();
			log_file << ss.str();
		};

		const bool clinet_has_time = ntp_client->has_time();
		if (acquired_time != clinet_has_time) {
			acquired_time = clinet_has_time;
			std::stringstream ss;
			ss  << "\t*******************************************************************************" << std::endl
				<< "\t\t " << (acquired_time ? "Acquired time" : "Lost time") << std::endl
				<< "\t*******************************************************************************" << std::endl;

			std::cout << ss.str();
			log_file << ss.str();
		};

		std::cout << "Sleeped (seconds) for " << kia.sleep(ntp_client).count() / 1000.0 << std::endl;
	};

	while (true)
	{
		try
		{
			queryProcedure();
		}
		catch (const std::exception& exc)
		{
			printf("%s", exc.what());
			break;
		}
	}
};

ntp::ClientsListManagerPtr start_init_clients_manager(bool include_local_ntp, ntp::IManager::ManagerSettings::ServersEvaluationMethod evalMthd, bool update_system_clock)
{
	static std::ofstream of("c:\\temp\\ClientsManager.log", std::ios::trunc);

	auto mng = ntp::IManager::create_instance();
	ntp::IManager::ManagerSettings setup;

	setup.clock_shelf_life = std::chrono::hours(24 * 365);
	setup.update_system_clock_on_clock_change = update_system_clock;

	//prefer spending longer time evaluating the best server in the list.:
	setup.servers_eval_method = evalMthd;

	setup.ntp_servers = compile_ntp_endpoints(include_local_ntp);
	//Do not alter the machines clock, use internal clock instead:
	setup.ncp_client_settings.lts = ntp::LocalTimeSource::InternalClock;

	//Conditions for inner clock update, that will overule the ClientSettings::ClockAdjustmentRules::min_clock_adjustment settings.
	//The idea is to keep the inner clock as updated as possible even if it did not qualfied for the minimal delta from the server.
	//-This is done if the network behaviour is stable and/or the deltas mesured from the server are stable(this implicitly holds network stability component).
	//-Note that on some systems, large intervals between query operation may lead to unstable network behaviour.
	setup.ncp_client_settings.minimal_clock_adjustment_rules.max_network_jitter = std::chrono::microseconds(750);
	setup.ncp_client_settings.minimal_clock_adjustment_rules.max_clock_adjustment_jitter = std::chrono::microseconds(750);

	//Setting the fine-tuning procedure stopping conditions:
	setup.ncp_client_settings.fta.clock_adjustment_jitter_min = std::chrono::milliseconds(1);
	setup.ncp_client_settings.fta.fine_adjustment_duration = std::chrono::seconds(60);

	setup.state_callback = [](const ntp::IManager& mng, ntp::IManager::State newState, void* userParam) ->void {
		std::stringstream oss; oss << '\t' << "ntp::IManager state changed:" << std::endl << newState.msg() << std::endl;
		std::cout << oss.str();
		of << oss.str();
		of.flush();
	};

	setup.server_evaluated_callback = [](const ntp::IManager& mng, ntp::EndPointPtr server, ntp::IManager::ServerRating rating, double grade, const ntp::Metrics & rating_metrics, std::string as_string, void* userParam) {
		std::stringstream oss; oss << '\t' << "ntp::IManager server evaluated:" << std::endl << as_string << std::endl << std::endl;
		of << oss.str();
		of.flush();
	};

	setup.request_result_callback = [](const ntp::IManager& mng, ntp::ResultPtr ntp_request_result, void* userParam) {
		std::stringstream oss;
		oss << '\t' << "ntp::IManager server query:" << std::endl;
		oss << "***********************************************************************************************************" << std::endl;
		oss<< ntp_request_result->as_string() << std::endl;
		oss << "***********************************************************************************************************" << std::endl;
		std::cout<< oss.str();
		of << oss.str();
		of.flush();
	};

	mng->begin_init(setup);
	return mng;
}

void test_clients()
{
	std::list<std::thread> threads;
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP(ntp::SpecialAddress::LOOPBACK_IPV4))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("pool.ntp.org"))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time.cloudflare.com"))));
	threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time.facebook.com"))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time.windows.com"))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time.google.com"))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time1.google.com"))));
	//threads.push_back(std::thread(execute_ntp_client_query_loop, ntp::EndPointPtr(new ntp::EndPointUDP("time.apple.com"))));





	while (!threads.empty()) {
		threads.front().join();
		threads.erase(threads.begin());
	}
}

void test_clients_manager(ntp::IManager::ManagerSettings::ServersEvaluationMethod evalMthd)
{
	auto mng = start_init_clients_manager(true, evalMthd, true);
	mng->wait_initialzed(std::chrono::milliseconds(-1));
	while (true)
	{
		//std::this_thread::sleep_for(std::chrono::seconds(8));
		//std::cout<< '\t'<< mng->get_time()->now().as_string() << std::endl;
		std::cout << '\t' << std::chrono::duration_cast<std::chrono::milliseconds>(mng->clock()->now().time().time_since_epoch()).count() << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::hours(10));
}

bool test_server(ntp::IManager::ManagerSettings::ServersEvaluationMethod evalMthd, bool use_external_time_source, bool listen_on_ipv6)
{
	static std::ofstream of("c:\\temp\\NetworkTimeServer.log", std::ios::trunc);
	auto ntp_server = ntp::IServer::create_instance();
	ntp::IServer::ServerSettings setup;

	if (use_external_time_source) {
		auto mng = start_init_clients_manager(false, evalMthd, true);
		mng->wait_initialzed(std::chrono::milliseconds(-1));
		setup.time_source = mng;
	}

	setup.error_callback = [](const ntp::IServer& server, std::string error_string, void* userParam) {
		std::stringstream oss; oss << "NTP server error: " << error_string << std::endl;
		std::cout << oss.str();
		of << oss.str();
		of.flush();
	};

	setup.warnning_callback = [](const ntp::IServer& server, std::string warn_string, void* userParam) {
		std::stringstream oss; oss << "NTP server warning: " << warn_string << std::endl;
		std::cout << oss.str();
		of << oss.str();
		of.flush();
	};

	setup.client_served_callback = [](const ntp::IServer& server, std::string client_addr, void* userParam) {
		std::stringstream oss; oss << server.inner_clock()->now().as_local_time_of_day_string()
								   << " ->NTP server served the client " << client_addr << std::endl;
		std::cout << oss.str();
		of << oss.str();
		of.flush();
	};


	const auto listen_addr_any = ntp::EndPointUDP(listen_on_ipv6 ? ntp::SpecialAddress::ADDR_ANY_IPV6 : ntp::SpecialAddress::ADDR_ANY_IPV4, ntp::UDP_PORT);
	if (!ntp_server->init(setup, listen_addr_any))
		return false;

	std::this_thread::sleep_for(std::chrono::hours(99999));
	return true;
}


void test_servers_diff()
{
	class clock_diff_test
	{
		void finetune_ntp_client()
		{
			ntp::ClientSettings cs;

			//Do not alter the machines clock, use internal clock instead:
			cs.lts = ntp::LocalTimeSource::InternalClock;

			//Setting the fine-tuning procedure stopping conditions:
			cs.fta.clock_adjustment_jitter_min = std::chrono::milliseconds(1);
			cs.fta.fine_adjustment_duration = std::chrono::seconds(60);

			cs.error_callback = [](const ntp::IClient& client, std::string error_string, void* userParam) {
				std::cout << "NTP client error: " << client.queried_server_endpoint()->as_string() << ' ' << error_string << std::endl;
			};

			cs.warnning_callback = [](const ntp::IClient& client, std::string warn_string, void* userParam) {
				std::cout << "NTP client warning: " << client.queried_server_endpoint()->as_string() << ' ' << warn_string << std::endl;
			};

			auto ntp_client = ntp::IClient::create_instance();
			ntp_client->init(cs);
			ntp_client->query(m_ep, false);
			ntp::ntp_client_kickoff_intervals_algorithm kia;
			while (!ntp_client->is_fine_tunning_over())
			{
				auto res = ntp_client->query(m_ep, false);
				if (res->status() == ntp::Status::ReceiveMsgTimeout)//stop on first timeout
					break;
				kia.sleep(ntp_client);
			}


			if (ntp::FineTuneOverReason::ReachedTargetClockAdjustment == ntp_client->finetune_over_reason())
				m_clock = ntp_client->clock();
		};

		std::thread m_worker;
		ntp::ManagedClockPtr m_clock;
		const ntp::EndPointPtr m_ep;

	public:
		clock_diff_test(ntp::EndPointPtr _ep) :m_ep(_ep) {}
		void start() {
			m_worker = std::thread(std::bind(&clock_diff_test::finetune_ntp_client, this));
		}
		void join() {
			m_worker.join();
		}

		ntp::ManagedClockPtr clock() const { return m_clock; }

		ntp::EndPointPtr ep() const { return m_ep; }
	};

	std::list<clock_diff_test> mc;

	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.google.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time1.google.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time2.google.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time3.google.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time4.google.com"))));

	/*mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time1.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time2.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time3.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time4.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time5.facebook.com"))));*/


	/*mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.apple.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.google.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.facebook.com"))));
	mc.push_back(clock_diff_test(ntp::EndPointPtr(new ntp::EndPointUDP("time.windows.com"))));*/

	for (auto& t : mc)	t.start();
	for (auto& t : mc)	t.join();

	for (auto it = mc.begin(); it != mc.end(); ++it)
	{
		if (!it->clock()) continue;
		for (auto nit = std::next(it); nit != mc.end(); ++nit)
		{
			if (!nit->clock()) continue;
			const auto diff = it->clock()->now().as_posix_time_microsec() - nit->clock()->now().as_posix_time_microsec();
			std::cout << " servers " << it->ep()->as_string() << " | " << nit->ep()->as_string() << " delta microsec: " << diff << std::endl;
		}
	}

}
