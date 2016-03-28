#ifndef YDREQUEST_H
#define YDREQUEST_H

#include "ydclient.h"
#include <string>
#include <boost/property_tree/ptree.hpp>
#include "general.h"
#include "ydprocess.h"

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
		ydNoError,
		ydDataParseError,
		ydOk
	    };

	    typedef struct 
	    {
		int error_code;
		std::string error_detail;
		std::string error_str;
	    } YdError;

	    YdRequest(std::string& token, boost::asio::io_service& ios, bool useSandbox,
		    YdProcess::Callback ydProcessCallback);
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

	    YdProcess::Callback ydProcessCallback_;
	    void runYdProcessCallback();

	    virtual void generateRequest() = 0;

	    void fetchError();
	    template<typename T>
		bool getNodeVal(boost::property_tree::ptree& pt, const char* path, T& val)
		{
		    using namespace boost::property_tree;
		    bool fetchOk = false;
		    T t;
		    try
		    {
			t = pt.get<T>(path);
			fetchOk = true;
		    }
		    catch(ptree_bad_data&)
		    {
			msyslog(LOG_ERR, "Got ptree_bad_data when requesting node at address %s", path);
		    }
		    catch(ptree_bad_path&)
		    {
			msyslog(LOG_ERR, "Got ptree_bad_path when requesting node at address %s", path);
		    }
		    if(fetchOk)
			val = t;
		    return fetchOk;
		}
    };
}

#endif /* YDREQUEST_H */
