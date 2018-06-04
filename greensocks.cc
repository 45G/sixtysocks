#include <stdlib.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <exception>
#include <system_error>
#include <sys/epoll.h>
#include <socks6util/socks6util.hh>

#include "core/poller.hh"
#include "proxifier/proxifier.hh"

using namespace std;

void usage()
{
	static const char *usageLines[] = {
		"usage: greensocks [-j <thread count>] [-o <cpu offset>]",
			"[-m <mode>]",
			"[-l <port>] [-t <tls port>]",
			"[-i]",
			"[-U <username>] [-P <password>]",
			"[-s <proxy IP>] [-p <proxy port>]",
		//TODO: TLS proxy
		NULL,
	};
	
	const char *line = usageLines[0];
	
	cerr << line << endl;
	line++;
	for (; line != NULL; line++)
		cerr << "\t" << line << endl;
	
	exit(EXIT_FAILURE);
}

enum Mode
{
	M_NONE,
	M_PROXIFIER,
	M_PROXY,
};

union SocketAddress
{
	sockaddr_storage storage;
	sockaddr_in ipv4;
	sockaddr_in6 ipv6;
};

int main(int argc, char **argv)
{
	char c;
	opterr = 0;
	int numThreads = thread::hardware_concurrency();
	int cpuOffset = 0;
	Mode mode = M_NONE;
	uint16_t port = 0;
	uint16_t tlsPort = 0;
	in_addr_t proxyIP;
	uint16_t proxyPort = 1080;
	SocketAddress proxy;
	SocketAddress tlsProxy;
	
	memset(&proxy.storage,    0, sizeof(sockaddr_storage));
	memset(&tlsProxy.storage, 0, sizeof(sockaddr_storage));
	
	//TODO: fix this shit
	while ((c = getopt(argc, argv, "j:o:m:l:t:i:U:P:s:p:")) != -1)
	{
		switch (c)
		{
		case 'j':
			numThreads = atoi(optarg);
			if (numThreads < 0)
				usage();
			break;
			
		case 'o':
			cpuOffset = atoi(optarg);
			if (cpuOffset < 0)
				usage();
			break;
			
		case 'm':
			if (string(optarg) == "proxfy")
				mode = M_PROXIFIER;
			else if (string(optarg) == "proxy")
				mode = M_PROXY;
			else
				usage();
			break;
			
		case 'l':
			port = atoi(optarg);
			if (port == 0)
				usage();
			break;
			
		case 't':
			tlsPort = atoi(optarg);
			if (tlsPort == 0)
				usage();
			break;
			
		case 'i':
			//TODO
			break;
			
		case 'U':
			//TODO
			break;
			
		case 'P':
			//TODO
			break;
			
		case 's':
			proxy.ipv4.sin_family = AF_INET;
			proxy.ipv4.sin_addr.s_addr = inet_addr(optarg);
			if (proxy.ipv4.sin_addr.s_addr == 0)
				usage();
			break;
			
		case 'p':
			proxyPort = atoi(optarg);
			if (proxyPort == 0)
				usage();
			break;
			
		default:
			usage();
		}
	}
	if (mode == M_NONE)
		usage();
	
	if (cpuOffset + numThreads > (int)thread::hardware_concurrency())
		usage();
	
	Poller poller(numThreads, cpuOffset);
	poller.start();
	
	int listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFD < 0)
		throw std::system_error(errno, std::system_category());
	
	// tolerable error
	static const int one = 1;
	setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(1234); 
	
	int rc = bind(listenFD, (struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	rc = listen(listenFD, 100);
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	rc = fcntl(listenFD, F_SETFD, O_NONBLOCK);
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
	
	poller.add(new Proxifier(proxy.storage, listenFD), listenFD, EPOLLIN);
	
	sleep(1000);
	
	poller.stop();
	
	return 0;
}
