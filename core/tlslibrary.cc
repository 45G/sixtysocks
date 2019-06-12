#include <nspr.h>
#include <nss.h>
#include <ssl.h>
#include <sslproto.h>
#include "tlsexception.hh"
#include "tlslibrary.hh"

using namespace std;

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
	SECStatus status = NSS_Init(configDir.c_str());
	if (status != SECSuccess)
		throw TLSException();
}

TLSLibrary::NSSLibrary::~NSSLibrary()
{
	NSS_Shutdown(); // might return error
}

static void tlsCheck(SECStatus status)
{
	if (status != SECSuccess)
		throw TLSException();
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
	//TODO: enable once I figure out why it messes everything up
	//tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA,       PR_TRUE));
}
