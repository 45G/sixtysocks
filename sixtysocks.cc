#include <stdlib.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <exception>
#include <system_error>
#include <sys/epoll.h>
#include <algorithm>
#include <socks6util/socks6util.hh>
#include "core/poller.hh"
#include "proxifier/proxifier.hh"
#include "proxy/proxy.hh"
#include "authentication/simplepasswordchecker.hh"
#include "tls/tlslibrary.hh"
#include "tls/tlscontext.hh"

using namespace std;
using boost::intrusive_ptr;

void usage()
{
	static const vector<string> USAGE_LINES = {
	//         12345678901234567890123456789012345678901234567890123456789012345678901234567890
		{ "usage: sixtysocks [-j <thread count>] [-o <cpu offset>]" },
		{         "[-m <mode>] (\"proxify\"/\"proxy\")" },
		{         "[-l <listen port>] [-t <TLS listen port>]" },
		{         "[-U <username>] [-P <password>]" },
		{         "[-s <proxy IP>] [-p <proxy port>]" },
		{         "[-C <certificate DB>] [-n <key nickname>] [-S <SNI>]" },
		{         "[-D] (defer request until socket is readable)" },
	};
	
	bool first = true;
	for (const string &line: USAGE_LINES)
	{
		if (!first)
			cerr << "\t";
		else
			first = false;
		
		cerr << line << endl;
	}
	
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
	int numThreads = 1;
	int cpuOffset = -1;
	Mode mode = M_NONE;
	uint16_t port = 0;
	uint16_t tlsPort = 0;
	uint16_t proxyPort = 1080;
	S6U::SocketAddress proxyAddr;
	bool useTLS = false;
	string username;
	string password;
	intrusive_ptr<SimplePasswordChecker> passwordChecker;
	bool defer = false;
	string certDB;
	string nick;
	string sni;

	//TODO: use stronger random (maybe /dev/urandom?)
	srand(time(nullptr));

	//TODO: fix this shit
	opterr = 0;
	char c;
	while ((c = getopt(argc, argv, "j:o:m:l:t:U:P:s:p:C:S:n:D")) != -1)
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
			
		case 'U':
			username = string(optarg);
			break;
			
		case 'P':
			password = string(optarg);
			break;
			
		case 's':
			proxyAddr.ipv4.sin_family      = AF_INET;
			proxyAddr.ipv4.sin_addr.s_addr = inet_addr(optarg);
			if (proxyAddr.ipv4.sin_addr.s_addr == 0)
				usage();
			break;
			
		case 'p':
			proxyPort = atoi(optarg);
			if (proxyPort == 0)
				usage();
			break;

		case 'C':
			certDB = string(optarg);
			useTLS = true;
			break;
			
		case 'S':
			sni = string(optarg);
			useTLS = true;
			break;
			
		case 'n':
			nick = string(optarg);
			useTLS = true;
			break;


		case 'D':
			defer = true;
			break;
			
		default:
			usage();
		}
	}
	if (mode == M_NONE)
		usage();

	proxyAddr.setPort(proxyPort);

	if (min(username.length(), password.length()) == 0 && max(username.length(), password.length()) > 0)
		usage();

	if (mode == M_PROXY && username.length() > 0)
		passwordChecker = new SimplePasswordChecker(username, password);

	if (!useTLS)
		tlsPort = 0;
	if (mode == M_PROXY && port == 0 && tlsPort == 0)
		usage();

	
//	if (cpuOffset + numThreads > (int)thread::hardware_concurrency())
//		usage();

	try
	{
		unique_ptr<TLSLibrary> tlsLibrary;
		unique_ptr<TLSContext> clientCtx;
		unique_ptr<TLSContext> serverCtx;
		
		if (useTLS)
		{
			tlsLibrary.reset(new TLSLibrary(certDB));

			if (mode == M_PROXIFIER)
				clientCtx.reset(new TLSContext(false, "",   sni));
			else /* M_PROXY */
				serverCtx.reset(new TLSContext(true,  nick, ""));
		}

		Poller poller(numThreads, cpuOffset);
		//poller.start();

		if (mode == M_PROXIFIER)
		{
			S6U::SocketAddress bindAddr;
			bindAddr.ipv4.sin_family      = AF_INET;
			bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
			bindAddr.ipv4.sin_port        = htons(port);

			poller.assign(new Proxifier(&poller, proxyAddr.storage, bindAddr, defer, username, password, clientCtx.get()));
		}
		else /* M_PROXY */
		{
			if (port != 0)
			{
				S6U::SocketAddress bindAddr;
				bindAddr.ipv4.sin_family      = AF_INET;
				bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
				bindAddr.ipv4.sin_port        = htons(port);

				poller.assign(new Proxy(&poller, bindAddr, passwordChecker.get(), nullptr));
			}
			if (tlsPort != 0)
			{
				S6U::SocketAddress bindAddr;
				bindAddr.ipv4.sin_family      = AF_INET;
				bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
				bindAddr.ipv4.sin_port        = htons(tlsPort);

				poller.assign(new Proxy(&poller, bindAddr, passwordChecker.get(), serverCtx.get()));
			}
		}

	//	sleep(1000);
		poller.threadFun(&poller);

		poller.stop();
		poller.join();
	}
	catch (exception &ex)
	{
		cerr << "Caught exception; exiting: " << ex.what() << endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
