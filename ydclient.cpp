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
	httpHeader_(useSandbox ? YdRemote::httpHeaderSandbox : YdRemote::httpHeader)
    {
	httpRequest_ += httpHeader_;
	httpRequest_ += to_string(request_.length()) + "\r\n";
	httpRequest_ += "Connection: close\r\n\r\n";
	httpRequest_ += request_ + "\r\n";

	cout << httpRequest_ << endl << "-------" << endl;

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
	    if(error != boost::asio::error::eof)
		msyslog(LOG_ERR, "%s", error.message().c_str());
	    std::cout << &httpResponse_ << endl;
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
}
