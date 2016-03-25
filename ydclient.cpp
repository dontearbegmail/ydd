#include "ydclient.h"
#include "general.h"
#include <boost/bind.hpp>
#include <iostream>

using namespace boost::asio;
using namespace std;

namespace ydd
{
    YdClient::YdClient(string& request, io_service& ios, bool useSandbox) :
	socket_(ios, YdRemote::ctx),
	hostIt_(useSandbox ? YdRemote::hostSandboxIt : YdRemote::hostIt),
	request_(request),
	useSandbox_(useSandbox),
	httpRequestHeader_(useSandbox ? YdRemote::httpHeaderSandbox : YdRemote::httpHeader),
	state_(inProgress)
    {
	httpRequest_ += httpRequestHeader_;
	httpRequest_ += to_string(request_.length()) + "\r\n";
	httpRequest_ += "Connection: close\r\n\r\n";
	httpRequest_ += request_ + "\r\n";

	async_connect(
		socket_.lowest_layer(), 
		hostIt_, 
		boost::bind(
		    &YdClient::handleConnect, 
		    this, 
		    boost::asio::placeholders::error
		    )
		);
    }

    void YdClient::handleConnect(const boost::system::error_code& error)
    {
	if(error)
	{
	    msyslog(LOG_ERR, "%s", error.message().c_str());
	    state_ = failed;
	    return;
	}
	socket_.async_handshake(
		ssl::stream_base::client,
		boost::bind(
		    &YdClient::handleHandshake, 
		    this,
		    boost::asio::placeholders::error
		    )
		);
    }

    void YdClient::handleHandshake(const boost::system::error_code& error)
    {
	if(error)
	{
	    msyslog(LOG_ERR, "%s", error.message().c_str());
	    state_ = failed;
	    return;
	}
	boost::asio::async_write(
		socket_,
		boost::asio::buffer(httpRequest_, httpRequest_.length()),
		boost::bind(
		    &YdClient::handleWrite, 
		    this,
		    boost::asio::placeholders::error));
    }

    void YdClient::handleWrite(const boost::system::error_code& error)
    {
	if(error)
	{
	    msyslog(LOG_ERR, "%s", error.message().c_str());
	    state_ = failed;
	    return;
	}
	boost::asio::async_read(
		socket_,
		httpResponse_,
		boost::asio::transfer_at_least(1),
		boost::bind(
		    &YdClient::handleRead, 
		    this,
		    boost::asio::placeholders::error));
    }

    void YdClient::handleRead(const boost::system::error_code& error)
    {
	if(error)
	{
	    if(error == boost::asio::error::eof || isShortRead(error))
	    {
		parseHttpResponse();
	    }
	    else
	    {
		msyslog(LOG_ERR, "%s", error.message().c_str());
		state_ = failed;
	    }
	    return;
	}
	boost::asio::async_read(
		socket_,
		httpResponse_,
		boost::asio::transfer_at_least(1),
		boost::bind(
		    &YdClient::handleRead, 
		    this,
		    boost::asio::placeholders::error));
    }

    void YdClient::parseHttpResponse()
    {
	state_ = failed;
	try
	{
	    istream response_stream(&httpResponse_);
	    if(!checkHttpVersionStatus(response_stream))
		return;
	    if(!flushHttpHeader(response_stream))
		return;
	    if(!parseHttpChunks(response_stream))
		return;
	}
	catch(istream::failure e)
	{
	    msyslog(LOG_ERR, "Got an exception while parsing an HTTP response: %s", e.what());
	}
	state_ = ok;
    }

    const string& YdClient::getJsonResponse()
    {
	return jsonResponse_;
    }

    bool YdClient::isShortRead(const boost::system::error_code& error)
    {
	if(error.category() == boost::asio::error::get_ssl_category())
	    if(error.value() == ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ))
		return true;
	return false;
    }

    bool YdClient::flushHttpHeader(istream& istr)
    {
	string str;
	bool gotRead = false;
	bool gotNl = false;

	do
	{
	    gotRead = getline(istr, str);
	    if(str[0] == '\r')
		gotNl = true;
	}
	while(gotRead && !gotNl);
	if(!gotNl)
	    msyslog(LOG_ERR, "Failed to parse HTTP header");
	return gotNl;
    }

    bool YdClient::checkHttpVersionStatus(istream& istr)
    {
	string httpVersion;
	unsigned int statusCode;
	istr >> httpVersion;
	istr >> statusCode;

	if(httpVersion != "HTTP/1.1")
	{
	    msyslog(LOG_ERR, "Response HTTP version is not HTTP/1.1, but %s", httpVersion.c_str());
	    return false;
	}
	if(statusCode != 200)
	{
	    msyslog(LOG_ERR, "Response status code is not 200, but %d", statusCode);
	    return false;
	}
	return true;
    }

    bool YdClient::parseHttpChunks(istream& istr)
    {
	string line;
	ssize_t chunkSize;
	vector<char> buf;
	jsonResponse_ = "";
	do
	{
	    getline(istr, line);
	    chunkSize = parseChunkSize(line);
	    if((chunkSize > 0) && istr)
	    {
		if(buf.size() < (size_t)chunkSize)
		{
		    buf.resize(chunkSize * 2);
		}
		istr.read(&buf[0], chunkSize);
		getline(istr, line); // chunkSize doesn't include terminating \r\n 
		jsonResponse_.append(buf.begin(), buf.begin() + chunkSize);
	    }
	}
	while((chunkSize > 0) && istr);
	if((chunkSize == 0) && istr)
	    return true;
	else
	    msyslog(LOG_ERR, "Error while reading chunks (last chunk size isn't 0 or data stream ended unexpectedly)");
	return false;
    }

    ssize_t YdClient::parseChunkSize(std::string& s)
    {
	ssize_t size = -1;
	if(s.empty())
	    return -1;
	if(s.back() == '\r')
	    s.pop_back();
	try
	{
	    size_t failchar = 0;
	    ssize_t tempsize = std::stoul(s, &failchar, 16);
	    if(failchar == s.length())
		size = tempsize;
	}
	catch(std::invalid_argument&)
	{
	    msyslog(LOG_ERR, "Got a wrong line instead of a chunk HEX size: %s", s.c_str());
	}
	catch(std::out_of_range&)
	{
	    msyslog(LOG_ERR, "Got out_of_range on the following chunk HEX size: %s", s.c_str());
	}
	return size;
    }

    YdClient::State YdClient::getState()
    {
	return state_;
    }
}
