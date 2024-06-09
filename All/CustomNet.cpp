#include "pch.h"
#include "CustomNet.h"
#include "test.h"

void FIGHT::CustomNet::sync_clocks()
{
	test_clients(threads);
	manager->set_time_fetcher(get_clock); 
	manager->set_start(0);
	while (manager->now() == 0) {}

	if (is_host) 
	{
		RAS::Time start_time = manager->now();
		send(start_time, '\xFF');
		manager->set_start(start_time);
	}


}
