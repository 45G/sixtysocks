#ifndef PASSWORDCHECKER_HH
#define PASSWORDCHECKER_HH

#include <string>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>

class PasswordChecker: public boost::intrusive_ref_counter<PasswordChecker>
{
public:
	virtual bool check(boost::shared_ptr<std::string> user, boost::shared_ptr<std::string> password) = 0;
	
	virtual ~PasswordChecker();
};

#endif // PASSWORDCHECKER_HH
