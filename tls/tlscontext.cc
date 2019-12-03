#include "tlscontext.hh"

void TLSContext::antiReplayCtxDeleter(SSLAntiReplayContext *antiReplayCtx)
{
	SSL_ReleaseAntiReplayContext(antiReplayCtx); //might return error
}
