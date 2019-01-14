#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>
#include <memory>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/intrusive_ptr.hpp>

class PasswordChecker: public boost::intrusive_ref_counter<PasswordChecker>
{
public:
	virtual bool check(std::shared_ptr<std::string> user, std::shared_ptr<std::string> password) = 0;
	
	virtual ~PasswordChecker();
};

#endif // PASSWORDCHECKER_HH
