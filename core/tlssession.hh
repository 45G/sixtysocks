#ifndef TLSSESSION_HH
#define TLSSESSION_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/intrusive_ptr.hpp>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include "spinlock.hh"

class TLSSession
{
	struct SessionData: public boost::intrusive_ref_counter<SessionData>
	{
		static const int MAX_TICKET_LEN = 256;

		WOLFSSL_SESSION *session;

		uint8_t ticket[MAX_TICKET_LEN];
		uint32_t ticketLen;

		SessionData()
			: session(NULL), ticketLen(0) {}
	};
	
	boost::intrusive_ptr<SessionData> sessionData;

public:
	TLSSession()
		: sessionData(new SessionData) {}

	void apply(WOLFSSL *tls)
	{
		boost::intrusive_ptr<SessionData> sessionData = this->sessionData;

		if (sessionData->session == NULL)
			return;
		wolfSSL_set_session(tls, sessionData->session); //tolerable error

		if (sessionData->ticketLen == 0)
			return;
		wolfSSL_set_SessionTicket(tls, sessionData->ticket, sessionData->ticketLen); //tolerable error
	}

	void update(WOLFSSL *tls)
	{
		boost::intrusive_ptr<SessionData> sessionData = new SessionData();

		sessionData->session = wolfSSL_get_session(tls);

		uint32_t ticketLen = SessionData::MAX_TICKET_LEN;
		int rc = wolfSSL_get_SessionTicket(tls, sessionData->ticket, &ticketLen);
		if (rc == SSL_SUCCESS)
			sessionData->ticketLen = ticketLen;

		this->sessionData = sessionData;
	}
};

#endif // TLSSESSION_HH
