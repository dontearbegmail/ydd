#include "ydrequest.h"
#include "ydclient.h"
#include <boost/property_tree/json_parser.hpp>
#include "general.h"

using namespace std;

namespace ydd
{
    YdRequest::YdRequest(string& token, boost::asio::io_service& ios, bool useSandbox) :
	token_(token),
	ios_(ios),
	useSandbox_(useSandbox),
	ydClient_(*this, ios, useSandbox),
	state_(init)
    {
	ydError_.error_code = 0;
	ydError_.error_str = "";
	ydError_.error_detail = "";
    }

    void YdRequest::run()
    {
	generateRequest();
	state_ = initError;
	try
	{
	    if(ptRequest_.empty())
		return;
	    ostringstream buf; 
	    write_json(buf, ptRequest_, false);
	    request_ = buf.str();
	}
	catch(std::exception& e)
	{
	    msyslog(LOG_ERR, "Got an exception while trying to generate a JSON request: %s", e.what());
	    return;
	}
	state_ = running;
	ydClient_.run();
    }

    void YdRequest::processResult()
    {
	if(ydClient_.getState() != YdClient::ok)
	{
	    state_ = runError;
	    return;
	}
	const string& json = ydClient_.getJsonResponse();
	if(json.empty())
	{
	    state_ = responseEmptyError;
	    return;
	}
	state_ = responseParse;
	try
	{
	    istringstream is(json);
	    read_json(is, ptResponse_);
	}
	catch(std::exception& e)
	{
	    state_ = responseParseError;
	    msyslog(LOG_ERR, "Got an exception while trying to parse the JSON request: %s", e.what());
	    return;
	}
	state_ = success;
	fetchError();
    }

    void YdRequest::fetchError()
    {
	using namespace boost::property_tree;
	if(state_ < success)
	    return;
	ptree::assoc_iterator codeIt = ptResponse_.find("error_code");
	if(codeIt != ptResponse_.not_found())
	{
	    state_ = ydError;
	    int code;
	    string detail, str;
	    try
	    {
		code = codeIt->second.get_value<int>();
		detail = ptResponse_.get<string>("error_detail");
		str = ptResponse_.get<string>("error_str");
	    }
	    catch(ptree_bad_data& e)
	    {
		code = 0;
		str = "";
		detail = "";
	    }
	    ydError_.error_code = code;
	    ydError_.error_str = str;
	    ydError_.error_detail = detail;
	}
	else
	{
	    state_ = ydNoError;
	}
    }

    string& YdRequest::get()
    {
	return request_;
    }

    const string& YdRequest::getJsonResponse()
    {
	return ydClient_.getJsonResponse();
    }

    YdRequest::State YdRequest::getState()
    {
	return state_;
    }

    const YdRequest::YdError& YdRequest::getYdError()
    {
	return ydError_;
    }

    void YdRequest::getYdErrorString(std::string& errStr)
    {
	errStr.clear();
	errStr.append("YDAPI error: error_code = ");
	errStr.append(std::to_string(ydError_.error_code));
	errStr.append(", error_str = \"");
	errStr.append(ydError_.error_str);
	errStr.append("\", error_detail = \"");
	errStr.append(ydError_.error_detail);
	errStr.append("\"");
    }

    bool YdRequest::getLongNode(boost::property_tree::ptree& pt, const char* path, long& val)
    {
	using namespace boost::property_tree;
	bool fetchOk = false;
	long t;
	try
	{
	    t = pt.get<long>(path);
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

    bool YdRequest::getStringNode(boost::property_tree::ptree& pt, const char* path, std::string& val)
    {
	using namespace boost::property_tree;
	bool fetchOk = false;
	std::string t;
	try
	{
	    t = pt.get<std::string>(path);
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
}
