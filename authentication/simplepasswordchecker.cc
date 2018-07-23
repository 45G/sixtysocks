#include "simplepasswordchecker.hh"

using namespace std;

SimplePasswordChecker::SimplePasswordChecker(const std::string &user, const std::string &password)
	: user(user), password(password)
{
	if (    user.size()     < 1 || user.size()     > 255 ||
		password.size() < 1 || password.size() > 255)
	{
		//TODO: proper exception
		throw exception();
	}
}

bool SimplePasswordChecker::check(boost::shared_ptr<string> user, boost::shared_ptr<string> password)
{
	if (user.get() == NULL)
		return false;
	return this->user == *user && this->password == *password;
}
