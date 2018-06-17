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
#include "proxy/proxy.hh"

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

int main(int argc, char **argv)
{
	char c;
	opterr = 0;
	int numThreads = 1;
	int cpuOffset = -1;
	Mode mode = M_NONE;
	uint16_t port = 0;
	uint16_t tlsPort = 0;
	uint16_t proxyPort = 1080;
	S6U::SocketAddress proxy;
	S6U::SocketAddress tlsProxy;
	bool idempotence = false;
	string username;
	string password;
	
	memset(&proxy.storage,    0, sizeof(sockaddr_storage));
	memset(&tlsProxy.storage, 0, sizeof(sockaddr_storage));
	
	//TODO: fix this shit
	while ((c = getopt(argc, argv, "j:o:m:l:t:iU:P:s:p:")) != -1)
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
			break;
			
		case 'm':
			if (string(optarg) == "proxify")
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
			idempotence = true;
			break;
			
		case 'U':
			username = string(optarg);
			break;
			
		case 'P':
			password = string(optarg);
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
	proxy.setPort(proxyPort);
	
//	if (cpuOffset + numThreads > (int)thread::hardware_concurrency())
//		usage();
	
	Poller poller(numThreads, cpuOffset);
	//poller.start();

	
	int listenFD = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (listenFD < 0)
		throw std::system_error(errno, std::system_category());
	
	// tolerable error
	static const int one = 1;
	setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port); 
	
	int rc = bind(listenFD, (struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	rc = listen(listenFD, 100);
	if (rc < 0)
		throw system_error(errno, std::system_category());
	
	// tolerable error
	S6U::Socket::saveSYN(listenFD);
	
	if (mode == M_PROXIFIER)
		poller.add(new Proxifier(&poller, proxy.storage, listenFD), listenFD, EPOLLIN);
	else
		poller.add(new Proxy(&poller, listenFD), listenFD, EPOLLIN);
	
//	sleep(1000);
	poller.threadFun(&poller);
	
	poller.stop();
	poller.join();
	
	return 0;
}
