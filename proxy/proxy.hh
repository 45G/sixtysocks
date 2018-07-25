#ifndef PROXY_HH
#define PROXY_HH

#include <memory>
#include <unordered_map>
#include <string>
#include <socks6util/socks6util.hh>
#include "../core/listenreactor.hh"
#include "../authentication/passwordchecker.hh"
#include "../authentication/lockabletokenstuff.h"

class Proxy: public ListenReactor
{
	std::unique_ptr<PasswordChecker> passwordChecker;
	
	std::unordered_map<std::string, std::unique_ptr<LockableTokenBank> > banks;
	Spinlock bankLock;

public:
	Proxy(Poller *poller, int listenFD, PasswordChecker *passwordChecker)
		: ListenReactor(poller, listenFD), passwordChecker(passwordChecker) {}
	
	void handleNewConnection(int fd);


	PasswordChecker *getPasswordChecker() const
	{
		return passwordChecker.get();
	}
	
	LockableTokenBank *createBank(const std::string &user, uint32_t size);
	
	LockableTokenBank *getBank(const std::string &user);
};

#endif // PROXY_HH
