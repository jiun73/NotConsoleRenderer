#include <algorithm>
#include <iostream>
#include <sstream>
#include <functional>
#include "ntp_manager.h"
#include "ntp_client.h"


using namespace ntp;


//////////////////

ntp::ClientsListManagerPtr ntp::IManager::create_instance() {
	return std::make_shared<ntp::Manager>();
}

ntp::Manager::Manager() : 
	 m_disposedEvt(false)
	,m_serverSelectedEvt(false)
	,m_force_oneshort_clock_updateEvt(false)
	,m_activeServerRating(ServerRating::Invalid)
	,m_clock(new ManagedClock())
	,m_is_monotonous_clock(false)
{
	ntp::init();

	if (m_settings.start_with_system_clock)
		m_clock.reset(new ManagedClock(std::chrono::system_clock::now(), std::chrono::steady_clock::now(), 0.0));
}

ntp::Manager::~Manager()
{
	m_disposedEvt.set();
	if (m_worker.joinable())
		m_worker.join();

	m_serverSelectedEvt.wait();

	m_settings.state_callback = nullptr;
	m_settings.server_evaluated_callback = nullptr;
}

void ntp::Manager::begin_init(const ManagerSettings & settings)
{
	m_settings = settings;	
	if (std::chrono::seconds(1) > m_settings.min_server_query_frequency)
		m_settings.min_server_query_frequency = std::chrono::seconds(1);

	if (m_settings.ntp_servers.empty()) {
		update_state(State(StatusCode::InvalidSettings, "InvalidSettings-> NTP servers list is empty."));
		m_serverSelectedEvt.set();
		return;
	}	

	update_state(State(ntp::IManager::StatusCode::Initializing, "Initializing->Initialization procedure is running."));
	m_worker = std::thread(std::bind(&ntp::Manager::manager_work, this));
}

ntp::IManager::State ntp::Manager::wait_initialzed(std::chrono::microseconds waitDuration) const
{
	if (0 <= waitDuration.count())
		m_serverSelectedEvt.wait(static_cast<size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(waitDuration).count()));
	else 
		m_serverSelectedEvt.wait();
	
	IManager::State stcpy;
	{
		std::shared_lock<std::shared_mutex> lock(m_server_lock);
		stcpy = m_state;
	}
	return stcpy;
}

IManager::State ntp::Manager::state(std::chrono::microseconds waitDuration) const
{
	IManager::State stcpy;
	{
		std::shared_lock<std::shared_mutex> lock(m_server_lock);
		stcpy = m_state;
	}
	return stcpy;
}

std::pair<ServerInfoPtr, IManager::ServerRating>  ntp::Manager::active_server() const
{
	std::pair<ServerInfoPtr, IManager::ServerRating> epr(ServerInfoPtr(), ServerRating::Invalid);
	{
		std::shared_lock<std::shared_mutex> lock(m_server_lock);
		if (m_activeServer)
			epr = std::make_pair(m_activeServer->server_info(), m_activeServerRating);
	}
	return epr;
}


bool ntp::Manager::has_time() const{
	bool res;
	{
		std::shared_lock<std::shared_mutex> lock(m_server_lock);
		res = (bool)m_clock;
	}
	return res;
}

//Returns time information. will return object that will not carry time if [has_time] returns false,
ManagedClockPtr ntp::Manager::clock() const
{
	ntp::ManagedClockPtr tm;
	{
		std::shared_lock<std::shared_mutex> lock(m_server_lock);
		tm = m_clock;
	}
	return tm;
}

bool ntp::Manager::is_clock_monotonous() const{	return m_is_monotonous_clock;}

void ntp::Manager::set_clock_as_monotonous(bool enable_monotonous){
	if (is_clock_monotonous() && !enable_monotonous)
		m_force_oneshort_clock_updateEvt.set();//if currently is monotonous and set not to, flag the one shot update event
	else
		m_force_oneshort_clock_updateEvt.reset();

	m_is_monotonous_clock = enable_monotonous;
}

void ntp::Manager::manager_work()
{
	std::chrono::steady_clock::time_point last_renew_time = std::chrono::steady_clock::time_point::min();
	const std::chrono::milliseconds MaxPoolIntervals = std::chrono::seconds(60);
	std::chrono::milliseconds poolIntervals = MaxPoolIntervals;

	const auto OnServerClockUpdate = [](ntp::Manager& mng) {
		//update the manager's time if the inner clock was updates.
		//1.One option to do it is to construct a clock manualy(this will use drift factor reguardless of the client's settings:
		//update_clock(ntp::ManagedClockPtr(new ManagedClock(res->time().time(), res->time_extraction_time(), res->mtr().drift_rate)));
		//2.Simpler option is to ask the remote server client, is updated clock. This will include the drift rate only id configured to such:
		auto clock = mng.m_activeServer->clock();
		mng.update_clock(clock);
		if (mng.m_settings.update_system_clock_on_clock_change)
			ntp::utils::set_system_clock(clock);
	};

	do
	{
		if (m_activeServer) 
		{
			ntp::ResultPtr requst_result = m_activeServer->query(nullptr, false);

			if (
				requst_result->used_for_setting_clock() &&
				(!is_clock_monotonous() || m_force_oneshort_clock_updateEvt.wait(0))
				)
			{
				m_force_oneshort_clock_updateEvt.reset();
				OnServerClockUpdate(*this);
			}

			if (m_settings.request_result_callback)
				m_settings.request_result_callback(*this, requst_result, m_settings.userParam);

		}
		
		bool renew = !m_activeServer;
		
		if (!renew && !is_clock_monotonous() && 0 < m_settings.clock_shelf_life.count() && 1 < m_settings.ntp_servers.size() &&
			//m_settings.clock_shelf_life < m_clock->duration_from_reference_time()
			//m_settings.clock_shelf_life < m_activeServer->get_duration_from_last_clock_update() 
			m_settings.clock_shelf_life < m_activeServer->duration_from_successful_query()
			)
		{
			update_state(State(ntp::IManager::StatusCode::ClockShelfLifeExpired, "ClockShelfLifeExpired-> renewing server"));
			renew = true;
		}

		if (!renew && m_activeServer->server_queries_success_rate() < m_settings.asd.min_availablitity  &&
			 last_renew_time != std::chrono::steady_clock::time_point::min()
			)
		{
			update_state(State(ntp::IManager::StatusCode::ServerAvailablitityDropped, "ServerAvailablitityDropped-> dropped to " + std::to_string(100 * m_activeServer->server_queries_success_rate())+ "%, renewing server"));
			renew = true;
		}

		if(renew)
		{
			ServerRating sr(ServerRating::Invalid);
			auto new_server = select_best_server(sr);
			{
				std::unique_lock<std::shared_mutex> lock(m_server_lock);
				const bool is_renewing = nullptr != m_activeServer.get();
				m_activeServer = new_server;
				m_activeServerRating = sr;
				last_renew_time = std::chrono::steady_clock::now();
			}

			if (m_activeServer) {				
				const auto srv = active_server();
				const char* const ServerRatingNames[]{ "Superb", "Excellent","Good","Satisfactory",	"Poor",	"Invalid" };
				std::ostringstream oss; oss << "NewServerObtained->Server was obtained: " << srv.first->as_string() << " | Rating: " << ServerRatingNames[(int)srv.second];
				update_state(State(ntp::IManager::StatusCode::NewServerObtained, oss.str()));
			}
			else {
				update_state(State(ntp::IManager::StatusCode::NoServer, "NoServer->Failed obtaining a server."));
				poolIntervals = MaxPoolIntervals;
			}

			if (m_activeServer) 
			{
				OnServerClockUpdate(*this);

				poolIntervals = std::max(m_settings.min_server_query_frequency, m_activeServer->server_info()->min_query_intervals);
				if (poolIntervals > MaxPoolIntervals)
					poolIntervals = MaxPoolIntervals;
			}

			m_serverSelectedEvt.set();

		}
	} while (!m_disposedEvt.wait(static_cast<size_t>(poolIntervals.count())));
}

void ntp::Manager::update_state(const State & st)
{
	{
		std::lock_guard<std::mutex> lock(m_state_lock);
		if (m_state.code() == st.code() && st.code() != StatusCode::ClockUpdated) return;
		m_state = st;
	}
	if (m_settings.state_callback)
		m_settings.state_callback(*this, m_state, m_settings.userParam);
}

void ntp::Manager::update_clock(ntp::ManagedClockPtr new_clock)
{
	std::ostringstream oss;
	oss << "ClockUpdated->Clock updated: "<< new_clock->now().as_local_time_string();

	if (has_time()) {
		const auto cur_tm = clock();
		auto offset_microsec = new_clock->now().as_posix_time_microsec() - cur_tm->now().as_posix_time_microsec();
		oss << " (milli offset " << offset_microsec/1000.0 << ')';
	}

	{
		std::unique_lock<std::shared_mutex> lock(m_server_lock);
		m_clock = new_clock;
	}
	update_state(State(ntp::IManager::StatusCode::ClockUpdated, oss.str()));
}

struct NTPRating {
	double grade;
	ntp::ClientPtr client;
	ntp::IManager::ServerRating rating;
	friend bool operator > (const NTPRating& l, const NTPRating& r){	return l.grade > r.grade;}
	friend bool operator < (const NTPRating& l, const NTPRating& r){	return l.grade < r.grade;}
};

static std::list<std::pair<ntp::utils::ping_result, EndPointPtr>> extract_best_ping_servers(std::list<EndPointPtr> eps, size_t best_nb, uint16_t pings_nb, std::chrono::milliseconds max_rountrip_delay)
{
	std::list<std::pair<ntp::utils::ping_result, EndPointPtr>>	results;
	struct ping_test {
		ntp::utils::ping_result pr;
		ntp::EndPointPtr server_ep;
		std::shared_ptr<std::thread> asynchWorker;
		uint16_t pings_nb;
		std::chrono::milliseconds max_rountrip_delay;

		ping_test(ntp::EndPointPtr ep, uint16_t _pings_nb, std::chrono::milliseconds _max_rountrip_delay) : 
			server_ep(ep), pings_nb(_pings_nb), max_rountrip_delay(_max_rountrip_delay){}
		~ping_test() { finish(); }

		void start(bool asynch) {
			auto pingtsk = [](ping_test* t) {ntp::utils::ping_ntp_server(t->server_ep, t->pings_nb, t->pr, t->max_rountrip_delay, std::chrono::milliseconds(50)); };
			if (asynch)
				asynchWorker.reset(new  std::thread(pingtsk, this));
			else pingtsk(this);
		}
		void finish() {
			if (asynchWorker && asynchWorker->joinable())
				asynchWorker->join();
		}

	};
	std::list<ping_test> ping_tests;
	for (auto& s : eps) {
		ping_tests.push_back(ping_test(s, pings_nb, max_rountrip_delay));
		ping_tests.back().start(true);
	}

	for (auto it = ping_tests.rbegin(); it != ping_tests.rend(); ++it)
		it->finish();

	auto bubbleSort = [](std::list<ping_test>& mylist) {
		if (2 > mylist.size())  return;
		bool swapped;
		do {
			swapped = false;
			for (auto it = mylist.begin(); it != std::prev(mylist.end()); ++it) {
				auto next = std::next(it);
				if (it->pr.median_roundtrip > next->pr.median_roundtrip) {
					std::swap(*it, *next);
					swapped = true;
				}
			}
		} while (swapped);
	};

	bubbleSort(ping_tests);

	if (0 == best_nb) best_nb = 1;
	for (auto it = ping_tests.begin(); it != ping_tests.end(); ++it) {
		if (!it->pr.sucess) continue;
		results.push_back(std::make_pair(it->pr, it->server_ep));
		if (results.size() == best_nb) break;
	}
	return results;
}

ntp::ClientPtr ntp::Manager::select_best_server(ServerRating &outRating) const
{
	const uint16_t NB_PINGS = 7;	
	outRating = ServerRating::Invalid;
	std::list<std::shared_ptr<ntp::IClient>> clients;
	std::list<std::thread> init_threads;

	std::map<std::string, ntp::utils::ping_result> ep_to_ping_result;
	
	//1.take the ones with most promising ping.
	const auto best_ping_servers = extract_best_ping_servers(m_settings.ntp_servers, 5, NB_PINGS, std::chrono::milliseconds(300));

	if (best_ping_servers.empty())
		return ClientPtr();

	const ManagerSettings::ServersEvaluationMethod eval_mtd = 1 == best_ping_servers.size() ? ManagerSettings::ServersEvaluationMethod::UltafastFast : m_settings.servers_eval_method;

	for (auto pr : best_ping_servers)
		ep_to_ping_result.insert(std::make_pair(pr.second->as_string(), pr.first));

	std::list<NTPRating> rating;
	if (ManagerSettings::ServersEvaluationMethod::UltafastFast == eval_mtd)
	{
		//creating the selection matrix.
		const double availablity = 0.25;
		const double net_response_time = 0.75;

		std::chrono::nanoseconds min_response_time(std::chrono::nanoseconds::max());
		for (auto ping_res : best_ping_servers)
		{
			if (ping_res.first.median_roundtrip < min_response_time)
				min_response_time = ping_res.first.median_roundtrip;
		}

		for (auto ping_res : best_ping_servers)
		{
			NTPRating ntpr;
			ntpr.rating = IManager::ServerRating::Invalid;
			ntpr.grade = availablity * (static_cast<double>(ping_res.first.roundtrip.size()) / static_cast<double>(NB_PINGS));
			ntpr.grade += net_response_time * (static_cast<double>(min_response_time.count()) / ping_res.first.median_roundtrip.count());
			ntpr.client = create_client(ping_res.second);
			if (!ntpr.client) continue;

			init_threads.push_back(std::thread(&Manager::init_server_com, this, ntpr.client, eval_mtd));

			if (ntpr.grade >= 0.95)
				ntpr.rating = IManager::ServerRating::Superb;
			else if (ntpr.grade >= 0.9)
				ntpr.rating = IManager::ServerRating::Excellent;
			else if (ntpr.grade >= 0.85)
				ntpr.rating = IManager::ServerRating::Good;
			else if (ntpr.grade >= 0.7)
				ntpr.rating = IManager::ServerRating::Satisfactory;
			else
				ntpr.rating = IManager::ServerRating::Poor;

			if (ping_res.first.median_roundtrip > std::chrono::milliseconds(75)){	
				if(IManager::ServerRating::Superb == ntpr.rating)
					ntpr.rating = IManager::ServerRating::Excellent;
			}
			if (ping_res.first.median_roundtrip > std::chrono::milliseconds(115)) {
				if (IManager::ServerRating::Excellent == ntpr.rating)
					ntpr.rating = IManager::ServerRating::Good;
			}
			if (ping_res.first.median_roundtrip > std::chrono::milliseconds(150)) {
				if (IManager::ServerRating::Good == ntpr.rating)
					ntpr.rating = IManager::ServerRating::Satisfactory;
			}
			else if (ping_res.first.median_roundtrip > std::chrono::milliseconds(200)) {
				if (IManager::ServerRating::Good == ntpr.rating)
					ntpr.rating = IManager::ServerRating::Poor;
			}
			rating.push_back(ntpr);
		}

		for (auto & t : init_threads)
			if (t.joinable())t.join();
	}
	else
	{
		for (auto& ep : best_ping_servers)
		{
			auto c = create_client(ep.second);
			if (!c) continue;
			clients.push_back(c);
			init_threads.push_back(std::thread(&Manager::init_server_com, this, c, eval_mtd));
		}

		for (auto & t : init_threads)
			if (t.joinable())t.join();

		//creating the selection matrix.
		const double availablity = 0.25;
		const double net_stability = 0.55;
		const double net_response_time = 0.15;
		const double poll_intervals = 0.05;

		double min_net_jitter_ns = std::numeric_limits<double>::max();
		double min_latency_ns = std::numeric_limits<double>::max();
		std::chrono::seconds min_poll_intervals_seconds(std::chrono::seconds::max());
		for (auto c_it = clients.begin(); c_it != clients.end(); )
		{
			if (!(*c_it)->has_time()) {
				c_it = clients.erase(c_it);
				continue;
			}

			if ((*c_it)->mtr()->ds.has_stats()) {
				min_net_jitter_ns = std::min(min_net_jitter_ns, (*c_it)->mtr()->ds.network_latency_jitter_ns);
				min_latency_ns = std::min(min_latency_ns, (*c_it)->mtr()->ds.network_latency_avg_ns);
			}

			min_poll_intervals_seconds = std::min(min_poll_intervals_seconds, (*c_it)->server_info()->min_query_intervals);

			++c_it;
		}

		if (clients.empty())
			return ClientPtr();

		for (const auto& c : clients)
		{
			NTPRating ntpr;
			ntpr.rating = IManager::ServerRating::Invalid;
			ntpr.client = c;
			ntpr.grade = availablity * c->server_queries_success_rate();
			auto mtr = c->mtr();
			if (mtr->ds.has_stats()) {
				ntpr.grade += net_stability * (min_net_jitter_ns / mtr->ds.network_latency_jitter_ns);//lower jitter -> higher grade
				ntpr.grade += net_response_time * (min_latency_ns / mtr->ds.network_latency_avg_ns);//lower latency -> higher grade
			}

			ntpr.grade += poll_intervals * (static_cast<double>(min_poll_intervals_seconds.count()) / c->server_info()->min_query_intervals.count());

			if (mtr->ds.has_stats())
			{
				const double jitter_millisec = mtr->ds.network_latency_jitter_ns / 1e6;

				if (c->server_queries_success_rate() >= 0.95)
				{
					if (jitter_millisec <= 0.5)
						ntpr.rating = IManager::ServerRating::Superb;
					else if (jitter_millisec <= 1)
						ntpr.rating = IManager::ServerRating::Excellent;
				}

				if (IManager::ServerRating::Invalid == ntpr.rating) {
					if (c->server_queries_success_rate() >= 0.8)
					{
						if (jitter_millisec <= 2)
							ntpr.rating = IManager::ServerRating::Good;
						else if (jitter_millisec <= 5)
							ntpr.rating = IManager::ServerRating::Satisfactory;
					}
				}
			}

			if (IManager::ServerRating::Invalid == ntpr.rating)
				ntpr.rating = IManager::ServerRating::Poor;

			rating.push_back(ntpr);
		}
		
	}//end of if(ManagerSettings::ServersEvaluationMethod::Comprehensive == m_settings.servers_eval_method)

	if (rating.empty())
		return ClientPtr();

	rating.sort(std::greater<NTPRating>());

	if (m_settings.server_evaluated_callback) {
		std::stringstream oss;
		for (const auto& r : rating) {
			auto res = r.client->last_clock_update_query_result();
			oss.clear(); oss.str("");
			oss << res->as_string();
			oss << "  [Grade: " << r.grade << "] Suceess rate: " << 100 * r.client->server_queries_success_rate() << '%';
			m_settings.server_evaluated_callback(*this, res->server(), r.rating, r.grade, *r.client->mtr(), oss.str(), m_settings.userParam);
		}
	}

	outRating = rating.front().rating;
	return rating.front().client;//return the best one
}

std::shared_ptr<ntp::IClient> ntp::Manager::create_client(EndPointPtr ep) const
{
	auto ntp_client = ntp::IClient::create_instance();
	ntp_client->init(m_settings.ncp_client_settings);
	auto r = ntp_client->query(ep, false);//test single call to query
	
	if(!r) return std::shared_ptr<ntp::IClient>();

	switch (r->status())
	{
	case ntp::Status::Ok:
	case ntp::Status::TimeAdjustLimitMin:
	case ntp::Status::ReceiveMsgTimeout://allow it to timeout on creation.
		return ntp_client;
	default:
		return std::shared_ptr<ntp::IClient>();
	}
}

void ntp::Manager::init_server_com(ntp::ClientPtr ntp_client, ManagerSettings::ServersEvaluationMethod mthd) const
{
	ntp_client_kickoff_intervals_algorithm kia;
	auto queryProcedure = [&ntp_client, &kia]() ->bool
	{
		auto ret= ntp_client->query(nullptr, false);
		kia.sleep(ntp_client);
		return ret && (ntp::Status::Ok == ret->status() || ntp::Status::TimeAdjustLimitMin == ret->status());
	};

	size_t successfull_calls = 0;
	while (!ntp_client->is_fine_tunning_over())	
	{
		try { 
			if (queryProcedure()) ++successfull_calls;
		}
		catch (const std::exception&){	break;	}

		switch (mthd)
		{
		case ntp::IManager::ManagerSettings::ServersEvaluationMethod::Comprehensive:
			break;
		case ntp::IManager::ManagerSettings::ServersEvaluationMethod::Fast:
			//returns immidiatly after the minimal stats collected 
			if ( ntp_client->mtr() && ntp_client->mtr()->ds.has_stats() && ntp_client->has_time()) return;
			break;
		case ntp::IManager::ManagerSettings::ServersEvaluationMethod::UltafastFast:
			//returns immidiatly after the 2nd query. This is the minimum as the 1st query will probebly show "massive" clock delta.
			if ( 1 < successfull_calls && ntp_client->has_time()) return;
		default:
			break;
		}
	}//end of while loop
}
