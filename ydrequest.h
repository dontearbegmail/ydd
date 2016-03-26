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
	    enum State
	    {
		init,
		initError,
		running,
		runError,
		responseEmptyError,
		responseParse,
		responseParseError,
		success, /* it means that the request was successfully sent to YD, 
			   a response was received and the response was successfully
			   parsed to ptResponse. But it doesn't mean a successful
			   completion of the requestd YD operation, i.e. for example
			   ptResponse may contain an error status from YD */
		ydError,
		ydNoError
	    };

	    typedef struct 
	    {
		int error_code;
		std::string error_detail;
		std::string error_str;
	    } YdError;

	    YdRequest(std::string& token, boost::asio::io_service& ios, bool useSandbox);
	    virtual void run();
	    virtual void processResult();
	    State getState();
	    const YdError& getYdError();
	    void getYdErrorString(std::string& errStr);

	    std::string& get();
	    const std::string& getJsonResponse();
	protected:
	    std::string token_;
	    boost::asio::io_service& ios_;
	    bool useSandbox_;
	    std::string request_;
	    YdClient ydClient_; 
	    const std::string locale_ = "ru";
	    State state_;
	    YdError ydError_;

	    boost::property_tree::ptree ptRequest_;
	    boost::property_tree::ptree ptResponse_;

	    void fetchError();
    };
}

#endif /* YDREQUEST_H */
