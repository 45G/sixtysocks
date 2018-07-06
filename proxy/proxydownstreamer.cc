#include <system_error>
#include "proxyupstreamer.hh"
#include "proxydownstreamer.hh"

using namespace std;
using namespace boost;

ProxyDownstreamer::ProxyDownstreamer(ProxyUpstreamer *upstreamer)
	: StreamReactor(upstreamer->getPoller(), -1, -1), state(S_INIT)
{
	//TODO: defer
	srcFD = dup(upstreamer->getDstFD());
	if (srcFD < 0)
		throw system_error(errno, system_category());

	dstFD = dup(upstreamer->getSrcFD());
	if (dstFD < 0)
	{
		close(srcFD); // tolerable error
		throw system_error(errno, system_category());
	}
}

void ProxyDownstreamer::enqueue(const S6M::AuthenticationReply *authRep)
{
	S6M::ByteBuffer bb(buf.getTail(), buf.availSize());
	authRep->pack(&bb);
	buf.use(bb.getUsed());
}

void ProxyDownstreamer::enqueue(S6M::OperationReply *opRep)
{

}
