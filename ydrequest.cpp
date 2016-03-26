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
	ydClient_(*this, ios, useSandbox)
    {
    }

    void YdRequest::run()
    {
	try
	{
	    if(this->ptRequest_.empty())
		return;
	    std::ostringstream buf; 
	    write_json(buf, ptRequest_, false);
	    request_ = buf.str();
	}
	catch(std::exception& e)
	{
	    msyslog(LOG_ERR, "Got an exception while trying to generate a JSON request: %s", e.what());
	    return;
	}
	//ydClient_ = YdClient::create(*this, ios_, useSandbox_);
    }

    void YdRequest::processResult()
    {
    }

    string& YdRequest::get()
    {
	return request_;
    }

}
