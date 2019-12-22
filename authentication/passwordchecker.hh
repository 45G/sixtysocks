#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>
#include <memory>

class PasswordChecker
{
public:
	virtual bool check(std::pair<const std::string *, const std::string *> creds) = 0;
	
	virtual ~PasswordChecker() = default;
};

#endif // PASSWORDCHECKER_HH
