#ifndef YDREQUEST_H
#define YDREQUEST_H

#include "ydclient.h"
#include <string>

namespace ydd
{
    class YdRequest
    {
	public:
	    //YdRequest(boost::asio::io_service& ios, bool useSandbox);
	    virtual void processResult();
	    std::string& get();
	protected:
	    std::string request_;
    };
}

#endif /* YDREQUEST_H */
