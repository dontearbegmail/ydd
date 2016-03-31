#ifndef YDDCONF_H
#define YDDCONF_H

#include <string>

namespace ydd
{
    class YddConf
    {
	public:
	    static std::string DbPrefix;
	    static std::string DbString;
	    static std::string DbUser;
	    static std::string DbPassword;
	    static const int DbMaxReconnectionAttempts = 5;
    };
}

#endif /* YDDCONF_H */
