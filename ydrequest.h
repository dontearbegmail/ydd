#ifndef YDREQUEST_H
#define YDREQUEST_H

#include "ydclient.h"
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace ydd
{
    class YdRequest
    {
	public:
	    YdRequest(std::string& token, boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	    std::string& get();
	protected:
	    std::string token_;
	    boost::asio::io_service& ios_;
	    bool useSandbox_;
	    std::string request_;
	    YdClient ydClient_; 
	    const std::string locale_ = "ru";

	    boost::property_tree::ptree ptRequest_;
    };
}

#endif /* YDREQUEST_H */
