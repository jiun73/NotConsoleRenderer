#ifndef __pinger_h__
#define __pinger_h__

#include <stdint.h>
#include <string>
#include <chrono>
#include <vector>

struct PingRequest {
	uint16_t id;
	uint8_t target_ip[64];
	int sequenceNumber;
	std::chrono::steady_clock::time_point tx;
	int receivedType;
	int receivedCode;
	int receivedTTL;//not available for IPv6
	std::chrono::steady_clock::time_point rx;
	std::string err_msg;
	bool receiving_failed;
	bool receiving_from_wrong_source;
	PingRequest() :
		tx(std::chrono::steady_clock::time_point::min())
		, rx(std::chrono::steady_clock::time_point::min())
		, receiving_failed(false), receiving_from_wrong_source(false), receivedTTL(-1)
	{}
};

class Pinger {
	

public:

	Pinger();
	~Pinger();

	//Checls and returns if the calling process/user has sufficiant privileges to use ping.
	static bool HasSufficiantPrivileges();

	bool Init(std::string target_host, int ttl = 64, std::chrono::milliseconds timeout = std::chrono::seconds(3));

	struct Result {
		bool success = false;
		std::chrono::nanoseconds round_trip = std::chrono::nanoseconds(0);
		int ttl = 0;
		int approximate_routers_crossed = 0;
		std::string err_msg;
	};

	std::vector<Result> Start(uint16_t nb_requests, std::chrono::nanoseconds requests_intervals);

private:
	void cleanup();
	bool send_ping_request(std::string destinationIp, uint16_t sequenceNumber, PingRequest &request);
	bool wait_For_ping_reply(uint16_t expectedSequenceNumber, PingRequest &pingRequest);

	bool send_ping_request_ipv4(std::string destinationIp, uint16_t sequenceNumber, PingRequest &request);
	bool send_ping_request_ipv6(std::string destinationIp, uint16_t sequenceNumber, PingRequest &request);

	bool wait_For_ping_reply_ipv4(uint16_t expectedSequenceNumber, PingRequest &pingRequest);
	bool wait_For_ping_reply_ipv6(uint16_t expectedSequenceNumber, PingRequest &pingRequest);

	int m_af_net;
	size_t m_sockfd;
	std::string m_target_ip_str;
	int m_ttl;
	std::chrono::milliseconds m_timeout;

};

#endif// __pinger_h__

