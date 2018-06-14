#ifndef PROXIFIERTFOPOLICY_HH
#define PROXIFIERTFOPOLICY_HH

#include <stdint.h>

struct ProxifierTFOPolicy
{
	enum Flags
	{
		F_TFO_SYN     = 1 << 0,
		F_SPEND_TOKEN = 1 << 1,
		F_TLS         = 1 << 2,
		F_TLS_0RTT    = 1 << 3,
		F_GOT_DATA    = 1 << 4,
	};

	static bool tfoPermitted(uint32_t flags);
};

#endif // PROXIFIERTFOPOLICY_HH
