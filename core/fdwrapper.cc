#include <system_error>
#include "fdwrapper.hh"

using namespace std;

RecvFDWrapper::RecvFDWrapper(SendFDWrapper sender)
	: FDWrapper(dup(sender.fd))
{
	if (fd < 0)
		throw system_error(errno, system_category());
}

SendFDWrapper::SendFDWrapper(RecvFDWrapper recver)
	: FDWrapper(dup(recver.fd))
{
	if (fd < 0)
		throw system_error(errno, system_category());
}
