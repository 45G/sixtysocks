#include <nspr.h>
#include <nss.h>
#include <ssl.h>
#include "tlsexception.hh"
#include "tlslibrary.hh"

TLSLibrary::NSPRLibrary::~NSPRLibrary()
{
	PR_Cleanup(); // might return error
}

TLSLibrary::NSSLibrary::NSSLibrary()
{
	SECStatus status = NSS_NoDB_Init(NULL);
	if (status != SECSuccess)
		throw TLSException();
}

TLSLibrary::NSSLibrary::~NSSLibrary()
{
	NSS_Shutdown(); // might return error
}

static void SECStatusCheck(SECStatus status)
{
	if (status != SECSuccess)
		throw TLSException();
}

TLSLibrary::Config::Config()
{
	SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_FDX, PR_TRUE));
	SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_SESSION_TICKETS, PR_TRUE));
	SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA, PR_TRUE));
	SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_FALSE_START, PR_TRUE));

	//TODO: what about:
	//SSL_ENABLE_FALLBACK_SCSV
	//SSL_ENABLE_EXTENDED_MASTER_SECRET
	//SSL_ENABLE_SIGNED_CERT_TIMESTAMPS
	//SSL_REQUIRE_DH_NAMED_GROUPS
	//SSL_ENABLE_TLS13_COMPAT_MODE
	//SSL_ENABLE_DTLS_SHORT_HEADER
	//SSL_ENABLE_HELLO_DOWNGRADE_CHECK
}
