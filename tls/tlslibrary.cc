#include <nspr.h>
#include <nss.h>
#include <ssl.h>
#include <sslproto.h>
#include <iostream>
#include <sslexp.h>
#include "tlsexception.hh"
#include "tlslibrary.hh"

using namespace std;

static void tlsCheck(SECStatus status)
{
	if (status != SECSuccess)
		throw TLSException();
}

TLSLibrary::NSPRLibrary::NSPRLibrary()
{
	PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);
}

TLSLibrary::NSPRLibrary::~NSPRLibrary()
{
	SSL_ClearSessionCache(); // might return error
	PR_Cleanup(); // might return error
}

TLSLibrary::NSSLibrary::NSSLibrary(const string &configDir)
{
	tlsCheck(NSS_Init(configDir.c_str()));
}

TLSLibrary::NSSLibrary::~NSSLibrary()
{
	NSS_Shutdown(); // might return error
}

TLSLibrary::TLSLibrary(const string &configDir)
	: nssLibrary(configDir)
{
	static const SSLVersionRange VER_RANGE = {
		.min = SSL_LIBRARY_VERSION_TLS_1_3,
		.max = SSL_LIBRARY_VERSION_TLS_1_3,
	};

	tlsCheck(SSL_VersionRangeSetDefault(ssl_variant_stream, &VER_RANGE));

	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_FDX,             PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_SESSION_TICKETS, PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_FALSE_START,     PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA,       PR_TRUE));

	static const int  SID_CACHE_ENTRIES = 1024;
	tlsCheck(SSL_ConfigServerSessionIDCache(SID_CACHE_ENTRIES, 0, 0, nullptr));

	//TODO: revisit anti-replay
#ifndef SSL_SetupAntiReplay_NotMandatory
	try
	{
#ifdef SSL_CreateAntiReplayContext
		/* nothing; done in TLSContext */
#else
#ifdef SSL_SetupAntiReplay
		static const int AR_WINDOW = 30;
		tlsCheck(SSL_SetupAntiReplay(AR_WINDOW * PR_USEC_PER_SEC, 7, 14));
#endif
#endif
	}
	catch (TLSException &ex)
	{
		cerr << ex.what() <<endl;
	}

#endif /* SSL_SetupAntiReplay_NotMandatory */
}
