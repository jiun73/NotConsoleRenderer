#ifndef __SOCKETS_HELPER_H__
#define __SOCKETS_HELPER_H__

#include <chrono>
#include <string>
#include <errno.h>

struct sockaddr;//forward declaration

namespace ntp {
namespace sockets{

    typedef size_t socket_t;

    extern const socket_t invalid_socket;
	extern const int socket_error;

	struct SocketError {
		int org_system_code;
		int posix_error_code;
		std::string msg;

		SocketError() : org_system_code(0), posix_error_code(0), msg(std::string()) {}

		//checks of the error is one of the given error codes.
		bool is_one_of(std::initializer_list<int> posix_codes) const;
		bool is(int posix_code) const;

		operator bool() const { return 0 == posix_error_code; }

		static SocketError get_last_error();
	};

    //returns true if the sockets library is initialized
    bool is_socket_lib_initialized();

    //initializes the sockets library
    bool socket_lib_init();

    int close_socket(ntp::sockets::socket_t sock);

    enum class shutdown_what { rx, tx, both };
    int shutdown_socket(ntp::sockets::socket_t sock, ntp::sockets::shutdown_what what);

    int set_socket_recv_timeout(ntp::sockets::socket_t sock, const std::chrono::milliseconds& timeout);

	int set_socket_shared_listening_addr_port(ntp::sockets::socket_t sock);

    int recvfrom(ntp::sockets::socket_t sock, char* buf, int len, int flags, sockaddr * in_addr, int* in_addr_len);

    int sendto(ntp::sockets::socket_t sock, const char* buf, int len, int flags, const sockaddr * to_addr, int to_addr_len);

    enum class addr_type { invalid, ipv4, ipv6 };

    enum class host_name_resolving_filter {
        any,
        prefer_ip_v4,
        prefer_ip_v6,
        ip_v4_only,
        ip_v6_only,
    };
    //Attempts to resolve the given host name, however if the string is an explicit valid IP, it will be returned as is.
    //On success returns string representation of the ip, and the type of the ip, oherwise returns empty string.
    //filter: controls the type of IP address that will be returned.
    //  -note, if [hostname] is an explicit valid IP formatted string, the "host_name_resolving_filter::prefer_ip_" values
    //   will not have any effect.
    std::string host_name_to_ip(std::string hostname, addr_type &out_addr_type, host_name_resolving_filter filter);

    //On success returns string representation of the ip, and the type of the ip, oherwise returns empty string.
    static inline std::string host_name_to_ip(std::string hostname, addr_type &out_addr_type) {
        return host_name_to_ip(hostname, out_addr_type, host_name_resolving_filter::any);
    }
}}

#endif//__SOCKETS_HELPER_H__
