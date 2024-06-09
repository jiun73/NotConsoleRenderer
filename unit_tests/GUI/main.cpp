//#include "../../ntp/include/intp_client.h"
//#include "../../ntp/include/intp_manager.h"
//#include "../../ntp/include/intp_server.h"
//
//#include <iostream>
//#include <sstream>
//#include <fstream>
//#include <conio.h>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//
//using namespace ntp;
//
//void run_ntp_client(std::shared_ptr<ntp::EndPointUDP> ep)
//{
//	std::ofstream of(std::string("c:\\temp\\") + ep->host + ".log", std::ios::trunc);
//
//	ntp::ClientSettings cs;
//	cs.lts = ntp::LocalTimeSource::InternalClock;//do not alter the machines clock, use internal clock instead.
//	
//	cs.error_callback = [](const ntp::IClient& client, std::string error_string, void* userParam) {
//		if (client.queried_server_endpoint())
//			std::cout << "NTP client error: " << client.queried_server_endpoint()->as_string() << ' ' << error_string << std::endl;
//		else
//			std::cout << "NTP client error: " << error_string << std::endl;
//	};
//
//	cs.warnning_callback = [](const ntp::IClient& client, std::string warn_string, void* userParam) {
//		std::cout << "NTP client warning: " << warn_string << std::endl;
//	};
//
//	cs.clockset_callback = [](const ntp::IClient& client, ManagedClockPtr new_clock, void* userParam) {
//		std::cout << "NTP client clockset: " << new_clock->now().as_local_time_string() << std::endl;
//	};
//
//	auto ntp_client = ntp::IClient::create_instance();
//	ntp_client->init(cs);
//
//	bool synched = false, acquired_time = false;
//	ntp_client_kickoff_intervals_algorithm kia;
//	auto queryProcedure = [&synched, &acquired_time, &ntp_client, &of, &ep, &kia]()
//	{
//		auto res =														 // main NTP server query
//			ntp_client->query(ep, false);
//
//		const bool ft_over = ntp_client->is_fine_tunning_over();
//		std::cout << res->as_string() << std::endl;// print status string
//		of << res->as_string() << std::endl;
//
//		if (synched != ft_over)
//		{
//			synched = ft_over;
//			std::stringstream ss;
//			ss << "\t**********************************************************" << std::endl
//				<< "\t\t " << (synched ? "Synched" : "Out of synch") << " Reason code: " << (int)ntp_client->finetune_over_reason() << std::endl
//				<< "\t**********************************************************" << std::endl;
//
//			std::cout << ss.str();
//			of << ss.str();
//		};
//
//		const bool clinet_has_time = ntp_client->has_time();
//		if (acquired_time != clinet_has_time) {
//			acquired_time = clinet_has_time;
//			std::stringstream ss;
//			ss << "\t*********************" << std::endl
//				<< "\t\t " << (acquired_time ? "Acquired time" : "Lost time") << std::endl
//				<< "\t*********************" << std::endl;
//
//			std::cout << ss.str();
//			of << ss.str();
//		};
//
//		std::cout << "Sleeped (seconds) for " << kia.sleep(ntp_client).count() / 1000.0 << std::endl;
//	};
//
//	while (!synched)
//	{
//		if (0 != _kbhit() && 'X' == toupper(_getch()))
//			break;
//
//		try
//		{
//			queryProcedure();
//		}
//		catch (const std::exception& exc)
//		{
//			printf("%s", exc.what());
//			break;
//		}
//	}
//
//	auto res = ntp_client->last_clock_update_query_result();
//	cv::Mat1b frame(cv::Size(1200, 300));
//	frame.setTo(cv::Scalar::all(0));
//
//	while (true) {
//
//		frame.setTo(cv::Scalar::all(0));
//		cv::putText(frame, res->now().as_local_time_string().c_str(), cv::Point(100, frame.rows/2), cv::FONT_HERSHEY_COMPLEX , 2, cv::Scalar::all(255), 2, cv::FILLED);
//		cv::imshow("Clock", frame);
//		cv::waitKey(1);
//	}
//};
//
//
//std::list<EndPointPtr> CompileEndpointsNTP(bool include_local_ntp) 
//{
//	std::list<EndPointPtr> ntp_servers;
//	if (include_local_ntp)
//		ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>(ntp::SpecialAddress::LOOPBACK_IPV4));
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.google.com"));//Google: poll intervals of 1
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time1.google.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time2.google.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time3.google.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time4.google.com"));
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.cloudflare.com"));// Cloudflare: poll intervals of 1
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.facebook.com"));//Facebook : poll intervals of 1
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time1.facebook.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time2.facebook.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time3.facebook.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time4.facebook.com"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time5.facebook.com"));
//
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.apple.com"));//Apple: poll intervals of 8
//
//	
//
//#if 0
//	//very low relability:
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("time.windows.com"));//Microsoft: poll intervals of 1
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("tik.cesnet.cz"));//poll intervals of 8
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("0.ubuntu.pool.ntp.org"));//Ubuntu community: poll intervals of 1
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("1.ubuntu.pool.ntp.org"));
//
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("0.nettime.pool.ntp.org"));//Netime project: poll intervals of 1
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("1.nettime.pool.ntp.org"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("2.nettime.pool.ntp.org"));
//	ntp_servers.push_back(std::make_shared<ntp::EndPointUDP>("3.nettime.pool.ntp.org"));
//
//
//	ntp_servers.push_back(EndPointPtr(new EndPointUDP("pool.ntp.org")));//poll intervals of 1
//	ntp_servers.push_back(EndPointPtr(new EndPointUDP("0.pool.ntp.org")));
//	ntp_servers.push_back(EndPointPtr(new EndPointUDP("1.pool.ntp.org")));
//	ntp_servers.push_back(EndPointPtr(new EndPointUDP("2.pool.ntp.org")));
//	ntp_servers.push_back(EndPointPtr(new EndPointUDP("3.pool.ntp.org")));
//#endif	
//	return ntp_servers;
//}
//
//ClientsListManagerPtr StartInitClientsManager(bool include_local_ntp)
//{
//	static std::ofstream of("c:\\temp\\ClientsManager.log", std::ios::trunc);
//
//	auto mng = ntp::IManager::create_instance();
//	ntp::IManager::ManagerSettings setup;
//
//	setup.clock_shelf_life = std::chrono::hours(24 * 365);
//
//	//prefer spending longer time evaluating the best server in the list.:
//	setup.servers_eval_method = ntp::IManager::ManagerSettings::ServersEvaluationMethod::Comprehensive;
//
//	setup.ntp_servers = CompileEndpointsNTP(include_local_ntp);
//	setup.ncp_client_settings.lts = ntp::LocalTimeSource::InternalClock;//do not alter the machines clock, use internal clock instead.
//
//	setup.ncp_client_settings.fta.clock_adjustment_jitter_min = std::chrono::milliseconds(1);
//	setup.ncp_client_settings.fta.fine_adjustment_duration = std::chrono::seconds(60);
//
//	setup.state_callback = [](const IManager& mng, IManager::State newState, void* userParam) ->void {
//		std::stringstream oss; oss << '\t' << "ntp::IManager state changed:" << std::endl << newState.msg() << std::endl;
//		std::cout << oss.str();
//		of << oss.str();
//		of.flush();
//	};
//
//	setup.server_evaluated_callback = [](const ntp::IManager& mng, EndPointPtr server, ntp::IManager::ServerRating rating, double grade, const ntp::Metrics & rating_metrics, std::string as_string, void* userParam) {
//		std::stringstream oss; oss << '\t' << "ntp::IManager server evaluated:" << std::endl << as_string << std::endl << std::endl;
//		of << oss.str();
//		of.flush();
//	};
//
//	mng->begin_init(setup);
//	return mng;
//}
//
//
//
//bool RunAsServer(bool use_external_time_source) {
//
//	auto ntp_server = ntp::IServer::create_instance();
//	ntp::IServer::ServerSettings setup;
//
//	if (use_external_time_source) {
//		auto mng = StartInitClientsManager(false);
//		mng->wait_initialzed(std::chrono::milliseconds(-1));
//		setup.time_source = mng;
//	}
//
//	setup.error_callback = [](const ntp::IServer& server, std::string error_string, void* userParam) {
//		std::cout << "NTP server error: " << error_string << std::endl;
//	};
//
//	setup.warnning_callback = [](const ntp::IServer& server, std::string warn_string, void* userParam) {
//		std::cout << "NTP server warning: " << warn_string << std::endl;
//	};
//
//	setup.client_served_callback = [](const ntp::IServer& server, std::string client_addr, void* userParam) {
//		std::cout << "NTP server served the client " << client_addr << std::endl;
//	};
//
//	if (!ntp_server->init(setup))
//		return false;
//
//
//	cv::Mat1b frame(cv::Size(1200, 300));
//	frame.setTo(cv::Scalar::all(0));
//
//	while (true) {
//
//		frame.setTo(cv::Scalar::all(0));
//		auto res = ntp_server->inner_clock();
//		cv::putText(frame, res->now().as_local_time_string().c_str(), cv::Point(100, frame.rows / 2), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar::all(255), 2, cv::FILLED);
//		cv::imshow("Clock", frame);
//		cv::waitKey(1);
//	}
//	
//	return true;
//}
//
//bool RunAsClient(std::string ip) {
//
//	run_ntp_client(std::shared_ptr<ntp::EndPointUDP>(new ntp::EndPointUDP(ip)) );
//	std::this_thread::sleep_for(std::chrono::hours(10));
//	return true;
//}
//
//int main()
//{
//	//RunAsClient("tik.cesnet.cz");
//	RunAsServer(false);
//
//	return 0;
//}