#ifndef AUTHUTIL_HH
#define AUTHUTIL_HH

#include <socks6msg/socks6msg.hh>
#include "proxy.hh"

namespace AuthUtil
{

S6M::AuthenticationReply authenticate(S6M::OptionSet *opts, Proxy *proxy);

}

#endif // AUTHUTIL_HH
