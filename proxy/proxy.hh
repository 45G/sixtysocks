#ifndef PROXY_HH
#define PROXY_HH

#include <memory>
#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"
#include "../authentication/passwordchecker.hh"

class Proxy: public ListenReactor
{
	std::unique_ptr<PasswordChecker> passwordChecker;

public:
	Proxy(Poller *poller, int listenFD, PasswordChecker *passwordChecker)
		: ListenReactor(poller, listenFD), passwordChecker(passwordChecker) {}
	
	void handleNewConnection(int fd);


	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
};

#endif // PROXY_HH
