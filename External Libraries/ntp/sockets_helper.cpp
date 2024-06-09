#include <string>
#include <cstring>
#include <vector>
#include <assert.h>
#include <exception>
#include <sstream>
#include <algorithm>

#include "sockets_helper.h"

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#pragma comment(lib,"Ws2_32.lib")
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
//linux
#include <sys/socket.h>
#include <unistd.h>//"close" function
#include <sys/time.h>//setting socket timeout
#include <netinet/in.h>
#include <arpa/inet.h>//inet_pton
#include <netdb.h>//addrinfo
#endif

#ifndef INVALID_SOCKET
const ntp::sockets::socket_t ntp::sockets::invalid_socket = ntp::sockets::socket_t(-1);
#else
const ntp::sockets::socket_t ntp::sockets::invalid_socket = ntp::sockets::socket_t(INVALID_SOCKET);
#endif

#ifndef SOCKET_ERROR
const int ntp::sockets::socket_error = int(-1);
#else
const int ntp::sockets::socket_error = int(SOCKET_ERROR);
#endif

static std::string error_to_string(int error_code)
{
	std::ostringstream oss; oss << error_code;

#ifdef WIN32
	char *s = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, NULL);

	oss << " [" << s << ']';
	LocalFree(s);
#else
	oss << " [" << strerror(error_code) << ']';
#endif
	return oss.str();
}

bool ntp::sockets::SocketError::is_one_of(std::initializer_list<int> posix_codes) const{
	std::vector<int> v(posix_codes);
	return v.end() != std::find(v.begin(), v.end(), posix_error_code);
}

 bool ntp::sockets::SocketError::is(int posix_code) const { return posix_error_code == posix_code; }

ntp::sockets::SocketError ntp::sockets::SocketError::get_last_error()
{
	SocketError e;

#ifdef WIN32
	auto winsock_erro_codes_to_posix = [](int winsockErrorCode) -> int
	{
		switch (winsockErrorCode) {
		case 0:				return 0;//no error
		case WSAEINTR:      return EINTR;
		case WSAEBADF:      return EBADF;
		case WSAEACCES:     return EACCES;
		case WSAEFAULT:     return EFAULT;
		case WSAEINVAL:     return EINVAL;
		case WSAEMFILE:     return EMFILE;
		case WSAEWOULDBLOCK: return EWOULDBLOCK;
		case WSAEINPROGRESS: return EINPROGRESS;
		case WSAEALREADY:   return EALREADY;
		case WSAENOTSOCK:   return ENOTSOCK;
		case WSAEDESTADDRREQ: return EDESTADDRREQ;
		case WSAEMSGSIZE:   return EMSGSIZE;
		case WSAEPROTOTYPE: return EPROTOTYPE;
		case WSAENOPROTOOPT: return ENOPROTOOPT;
		case WSAEPROTONOSUPPORT: return EPROTONOSUPPORT;
		case WSAEOPNOTSUPP: return EOPNOTSUPP;
		case WSAEAFNOSUPPORT: return EAFNOSUPPORT;
		case WSAEADDRINUSE: return EADDRINUSE;
		case WSAEADDRNOTAVAIL: return EADDRNOTAVAIL;
		case WSAENETDOWN:   return ENETDOWN;
		case WSAENETUNREACH: return ENETUNREACH;
		case WSAENETRESET:  return ENETRESET;
		case WSAECONNABORTED: return ECONNABORTED;
		case WSAECONNRESET: return ECONNRESET;
		case WSAENOBUFS:    return ENOBUFS;
		case WSAEISCONN:    return EISCONN;
		case WSAESHUTDOWN:   return ENOTCONN;
		case WSAENOTCONN:   return ENOTCONN;
		case WSAETIMEDOUT:  return EAGAIN /*ETIMEDOUT*/;
		case WSAECONNREFUSED: return ECONNREFUSED;
		case WSAELOOP:      return ELOOP;
		case WSAENAMETOOLONG: return ENAMETOOLONG;
		case WSAEHOSTUNREACH: return EHOSTUNREACH;
		case WSAENOTEMPTY:  return ENOTEMPTY;
		default:            return 1;  // Unknown error code
		}
	};

	e.org_system_code = WSAGetLastError();
	e.posix_error_code = winsock_erro_codes_to_posix(e.org_system_code);

#else
	e.posix_error_code = e.org_system_code = e.posix_error_code = errno;

#endif// WIN32

    if (!e)
        e.msg = error_to_string(e.org_system_code);

	return e;
}

bool ntp::sockets::is_socket_lib_initialized()
{
	ntp::sockets::socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == ntp::sockets::socket_error) {
		return false;
	}

	close_socket(s);
	return true;
}

bool ntp::sockets::socket_lib_init()
{
	if (is_socket_lib_initialized()) return true;

#ifdef WIN32
	WSADATA wsa{ 0 };
	return 0 == (WSAStartup(MAKEWORD(2, 2), &wsa));
#else
	return true;
#endif//WIN32
}

int ntp::sockets::close_socket(ntp::sockets::socket_t sock)
{
	//linux sockets compatability:
#ifdef WIN32
	return closesocket(sock);
#else
	return close(sock);
#endif // WIN32
}

int ntp::sockets::shutdown_socket(ntp::sockets::socket_t sock, ntp::sockets::shutdown_what what) {
	//linux sockets compatability:
#ifdef WIN32
	switch (what) {
	case ntp::sockets::shutdown_what::both: return shutdown(sock, SD_BOTH);
	case ntp::sockets::shutdown_what::rx: return shutdown(sock, SD_RECEIVE);
	case ntp::sockets::shutdown_what::tx: return shutdown(sock, SD_SEND);
	default: throw std::exception();
	}
#else
	switch (what) {
	case ntp::sockets::shutdown_what::both: return shutdown(sock, SHUT_RDWR);
	case ntp::sockets::shutdown_what::rx: return shutdown(sock, SHUT_RD);
	case ntp::sockets::shutdown_what::tx: return shutdown(sock, SHUT_WR);
	default: throw std::exception();
	}
#endif // WIN32
}


int ntp::sockets::set_socket_recv_timeout(ntp::sockets::socket_t sock, const std::chrono::milliseconds& timeout)
{
#ifdef WIN32
	const int timeout_val = static_cast<int>(timeout.count());
#else
	timeval timeout_val;
	timeout_val.tv_sec = static_cast<int64_t>(timeout.count()) / 1000;
	timeout_val.tv_usec = static_cast<int64_t>(((static_cast<int64_t>(timeout.count()) % 1000) / 1000.0) * 1000000);
#endif
	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_val, sizeof(timeout_val));
}

int ntp::sockets::set_socket_shared_listening_addr_port(ntp::sockets::socket_t sock)
{
	// enable the address reusage option
	int socket_opt_value = 1;//allow sharing the port/address
	if (ntp::sockets::socket_error == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&socket_opt_value, sizeof(socket_opt_value)))
		return ntp::sockets::socket_error;

#ifdef WIN32
	socket_opt_value = 0;//disable exlucive port/address
	return setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&socket_opt_value, sizeof(socket_opt_value));
#else
	socket_opt_value = 1;//disable exlucive port/address
	return setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *)&socket_opt_value, sizeof(socket_opt_value));
#endif
}

int ntp::sockets::recvfrom(ntp::sockets::socket_t sock, char * buf, int len, int flags, sockaddr * in_addr, int * in_addr_len)
{
#ifdef WIN32
	int addr_length = in_addr_len ? *in_addr_len : 0;
#else
	socklen_t addr_length = static_cast<socklen_t>(in_addr_len ? *in_addr_len : int(0));
#endif

	const int res = ::recvfrom(sock, buf, len, flags, in_addr, &addr_length);
	if (in_addr_len) *in_addr_len = static_cast<int>(addr_length);
	return res;
}

int ntp::sockets::sendto(ntp::sockets::socket_t sock, const char* buf, int len, int flags, const sockaddr * to_addr, int to_addr_len)
{
#ifdef WIN32
	const int addr_length = to_addr_len;
#else
	const socklen_t addr_length = static_cast<socklen_t>(to_addr_len);
#endif
	return ::sendto(sock, buf, len, flags, to_addr, to_addr_len);
}

/////////////////////////// HOST NAME _RESOLVING ///////////////////////////
static bool isSubnet(const sockaddr_in& address) {
	// Extract the address from sockaddr_in6
	const in_addr& addressIP = address.sin_addr;
	// Check if the 7th and 8th bytes are not zero
	return ((uint8_t*)&addressIP)[3] == 0;
}

static bool isIPv6Subnet(const std::string& ipv6Address) {
	std::vector<std::string> tokens;
	std::istringstream iss(ipv6Address);
	std::string token;

	while (std::getline(iss, token, ':'))
		tokens.push_back(token);

	// Check if the last token is empty, indicating double colons (::)
	if (tokens.back().empty()) {
		// If it is empty, it's likely a subnet
		return true;
	}
	else {
		// If the last token is not empty, it's likely a specific host
		return false;
	}
}

std::string ntp::sockets::host_name_to_ip(std::string hostname, addr_type &out_addr_type, host_name_resolving_filter filter)
{
	enum class resolving_preference {
		ignore_subnets,
		prefer_hosts,
	}const preference = resolving_preference::prefer_hosts;


	ntp::sockets::socket_lib_init();
	out_addr_type = addr_type::invalid;

	{
		sockaddr_in addr_v4{ 0 };	addr_v4.sin_family = AF_INET;	addr_v4.sin_port = 1234;//dummy port
		if (1 == inet_pton(addr_v4.sin_family, hostname.c_str(), &addr_v4.sin_addr)) {
			//already ip v4.
			if (host_name_resolving_filter::ip_v6_only == filter)
				return std::string();

			out_addr_type = addr_type::ipv4;
			return hostname;
		}

		sockaddr_in6 addr_v6{ 0 }; addr_v6.sin6_family = AF_INET6; addr_v6.sin6_port = 1234;//dummy port
		if (1 == inet_pton(addr_v6.sin6_family, hostname.c_str(), &addr_v6.sin6_addr)) {
			//already ip v6.
			if (host_name_resolving_filter::ip_v4_only == filter)
				return std::string();

			out_addr_type = addr_type::ipv6;
			return hostname;
		}
	}

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	switch (filter)
	{
	case ntp::sockets::host_name_resolving_filter::prefer_ip_v6:
	case ntp::sockets::host_name_resolving_filter::prefer_ip_v4:
	case ntp::sockets::host_name_resolving_filter::any:
		hints.ai_family = AF_UNSPEC;
		break;
	case ntp::sockets::host_name_resolving_filter::ip_v4_only:
		hints.ai_family = AF_INET;// AF_INET6
		break;
	case ntp::sockets::host_name_resolving_filter::ip_v6_only:
		hints.ai_family = AF_INET6;
		break;
	default:
		throw std::exception();
		break;
	}

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;//for what ever reason this only works with TCP on linux.
	addrinfo *result = nullptr;
	if (0 != getaddrinfo(hostname.c_str(), NULL, &hints, &result))
		return std::string();

	std::string first_found_ip_v6, first_found_ip_v4, first_found_ip_v6_subnet, first_found_ip_v4_subnet;

	// Retrieve each address and take the 1st one.
	for (auto ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		switch (ptr->ai_family)
		{
		case AF_INET:
		{
			if (!first_found_ip_v4.empty()) continue;

			sockaddr_in  *sockaddr_ipv4 = (sockaddr_in *)ptr->ai_addr;
			//Determine whether the address is a subnet address or a host address
			const bool is_subnet = isSubnet(*sockaddr_ipv4);
			if (is_subnet) {
				if (resolving_preference::ignore_subnets == preference) 	continue;
				if (!first_found_ip_v4_subnet.empty()) continue;
			}

			char ip_str[256] = { '\0' };
			inet_ntop(ptr->ai_addr->sa_family, &sockaddr_ipv4->sin_addr, ip_str, sizeof(ip_str));

			if (is_subnet)
				first_found_ip_v4_subnet = ip_str;
			else
				first_found_ip_v4 = ip_str;
			break;
		}
		case AF_INET6:

			if (!first_found_ip_v6.empty()) continue;

			sockaddr_in6  *sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			char ip_str[256] = { '\0' };
			inet_ntop(ptr->ai_addr->sa_family, &sockaddr_ipv6->sin6_addr, ip_str, sizeof(ip_str));
			//Determine whether the address is a subnet address or a host address

			const bool is_subnet = isIPv6Subnet(ip_str);
			if (is_subnet) {
				if (resolving_preference::ignore_subnets == preference)	continue;
				if (!first_found_ip_v6_subnet.empty()) continue;
			}

			if (is_subnet)
				first_found_ip_v6_subnet = ip_str;
			else
				first_found_ip_v6 = ip_str;
			break;
		}//end of switch

		if (!first_found_ip_v4.empty() && !first_found_ip_v6.empty()) break;//found one of each, break the loop

	}//end of for loop
	freeaddrinfo(result);

	if (first_found_ip_v4.empty())
		first_found_ip_v4 = first_found_ip_v4_subnet;

	if (first_found_ip_v6.empty())
		first_found_ip_v6 = first_found_ip_v6_subnet;

	switch (filter)
	{
	case ntp::sockets::host_name_resolving_filter::ip_v4_only: {
		out_addr_type = addr_type::ipv4;
		return first_found_ip_v4;
	}
	case ntp::sockets::host_name_resolving_filter::ip_v6_only: {
		out_addr_type = addr_type::ipv6;
		return first_found_ip_v6;
	}
	case ntp::sockets::host_name_resolving_filter::prefer_ip_v4:
		if (!first_found_ip_v4.empty()) {
			out_addr_type = addr_type::ipv4;
			return first_found_ip_v4;
		}
		break;
	case ntp::sockets::host_name_resolving_filter::prefer_ip_v6:
		if (!first_found_ip_v6.empty()) {
			out_addr_type = addr_type::ipv6;
			return first_found_ip_v6;
		}
		break;
	}
	//so far the prefered/requested  where not returned

	if (!first_found_ip_v4.empty()) {
		out_addr_type = addr_type::ipv4;
		return first_found_ip_v4;
	}

	if (!first_found_ip_v6.empty()) {
		out_addr_type = addr_type::ipv6;
		return first_found_ip_v6;
	}

	return std::string();
}

////////////////////////////////////////////////////////////////////////////


