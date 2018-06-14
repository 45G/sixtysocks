#include "proxifiertfopolicy.hh"

bool ProxifierTFOPolicy::tfoPermitted(uint32_t flags)
{
	/* client tolerates TFO */
	if (flags & F_TFO_SYN)
		return true;

	/* got idempotence */
	if (flags & F_SPEND_TOKEN)
		return true;

	/* no application data */
	if (!(flags & F_GOT_DATA))
		return true;

	/* TLS w/o 0-RTT data */
	if ((flags & F_TLS) && !(flags & F_TLS_0RTT))
		return true;

	return false;
}
