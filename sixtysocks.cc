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
#include <socks6util/socks6util.hh>
#include <algorithm>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "core/poller.hh"
#include "proxifier/proxifier.hh"
#include "proxy/proxy.hh"
#include "authentication/simplepasswordchecker.hh"
#include "core/sslexception.hh"

using namespace std;

void usage()
{
	static const char *usageLines[] = {
		"usage: sixtysocks [-j <thread count>] [-o <cpu offset>]",
			"[-m <mode>] (\"proxify\"/\"proxy\")",
			"[-l <listen port>] [-t <TLS listen port>]",
			"[-U <username>] [-P <password>]",
			"[-s <proxy IP>] [-p <proxy port>]",
			"[-V <trusted certificate file>]"
			"[-C <certificate file>]"
			"[-K <key file>]",
		NULL,
	};
	
	const char **line = &usageLines[0];
	
	cerr << *line << endl;
	line++;
	while (*line != NULL)
	{
		cerr << "\t" << *line << endl;
		line++;
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
	char c;
	opterr = 0;
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
	boost::intrusive_ptr<SimplePasswordChecker> passwordChecker;
	string veriFile;
	string certFile;
	string keyFile;
	
	srand(time(NULL));

	//TODO: fix this shit
	while ((c = getopt(argc, argv, "j:o:m:l:t:U:P:s:p:V:C:K:")) != -1)
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

		case 'V':
			veriFile = string(optarg);
			useTLS = true;
			break;

		case 'C':
			certFile = string(optarg);
			useTLS = true;
			break;

		case 'K':
			keyFile = string(optarg);
			useTLS = true;
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

	/* OpenSSL */
	if (useTLS)
	{
		static const int CERT_VERIFY_DEPTH = 3;

		int rc;

		SSL_library_init();
		SSL_load_error_strings();
		SSL_CTX *ctx = SSL_CTX_new(TLS_method());
		if (ctx == NULL)
			throw SSLException(ERR_get_error());

		SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_ANTI_REPLAY | SSL_OP_NO_RENEGOTIATION);
		SSL_CTX_set_verify_depth(ctx, CERT_VERIFY_DEPTH);

		if (veriFile.length() > 0)
		{
			rc = SSL_CTX_load_verify_locations(ctx, veriFile.c_str(), NULL);
			if (rc == 0)
				throw SSLException(ERR_get_error());
		}

		if (certFile.length() > 0)
		{
			rc = SSL_CTX_use_certificate_file(ctx, certFile.c_str(),  SSL_FILETYPE_PEM);
			if (rc == 0)
				throw SSLException(ERR_get_error());
		}

		if (keyFile.length() > 0)
		{
			rc = SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM);
			if (rc == 0)
				throw SSLException(ERR_get_error());
		}
	}

	Poller poller(numThreads, cpuOffset);
	//poller.start();
	
	if (mode == M_PROXIFIER)
	{
		S6U::SocketAddress bindAddr;
		bindAddr.ipv4.sin_family      = AF_INET;
		bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
		bindAddr.ipv4.sin_port        = htons(port);

		poller.assign(new Proxifier(&poller, proxyAddr.storage, bindAddr, username, password));
	}
	else /* M_PROXY */
	{
		if (port != 0)
		{
			S6U::SocketAddress bindAddr;
			bindAddr.ipv4.sin_family      = AF_INET;
			bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
			bindAddr.ipv4.sin_port        = htons(port);

			poller.assign(new Proxy(&poller, bindAddr, passwordChecker.get()));
		}
		if (tlsPort != 0)
		{
			S6U::SocketAddress bindAddr;
			bindAddr.ipv4.sin_family      = AF_INET;
			bindAddr.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
			bindAddr.ipv4.sin_port        = htons(tlsPort);

			//TODO
		}
	}
	
//	sleep(1000);
	poller.threadFun(&poller);
	
	poller.stop();
	poller.join();
	
	return 0;
}
