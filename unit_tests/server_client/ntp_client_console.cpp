
#include <iostream>
#include <thread>
#include <list>

#include "tests.h"

int main(int argc, char** argv)
{
	//test_servers_diff();
	//test_server(ntp::IManager::ManagerSettings::ServersEvaluationMethod::Fast, true, true);//test server on ipv6
	//test_server(ntp::IManager::ManagerSettings::ServersEvaluationMethod::Fast, true, false);//test server on ipv4
	//test_clients();
	test_clients_manager(ntp::IManager::ManagerSettings::ServersEvaluationMethod::Fast);

	//test_server(ntp::IManager::ManagerSettings::ServersEvaluationMethod::UltafastFast, true, false);//test server on ipv4

	return 0;
}

