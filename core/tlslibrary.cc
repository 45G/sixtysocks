#include <nspr.h>
#include <nss.h>
#include <ssl.h>
#include "tlsexception.hh"
#include "tlslibrary.hh"

TLSLibrary::NSPRLibrary::NSPRLibrary()
{
	PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);
}

TLSLibrary::NSPRLibrary::~NSPRLibrary()
{
	SSL_ClearSessionCache(); // might return error
	PR_Cleanup(); // might return error
}

TLSLibrary::NSSLibrary::NSSLibrary()
{
	SECStatus status = NSS_NoDB_Init(NULL);
	if (status != SECSuccess)
		throw TLSException();
}

TLSLibrary::NSSLibrary::NSSLibrary(const std::string &configDir)
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

TLSLibrary::TLSLibrary(const std::string &configDir)
	: nssLibrary(configDir)
{
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_FDX, PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_SESSION_TICKETS, PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_FALSE_START, PR_TRUE));
	tlsCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA, PR_TRUE));
}
