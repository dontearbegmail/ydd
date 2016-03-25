#ifndef YDCLIENT_H
#define YDCLIENT_H

#include "ydremote.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

namespace ydd
{
    class YdClient
    {
	public: 
	    YdClient(std::string& request, boost::asio::io_service& ios, bool useSandbox);
	    void handleConnect(const boost::system::error_code& error);
	    void handleHandshake(const boost::system::error_code& error);
	    void handleWrite(const boost::system::error_code& error);
	    void handleRead(const boost::system::error_code& error);
	    void parseHttpResponse();
	private:
	    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	    boost::asio::ip::tcp::resolver::iterator& hostIt_;
	    std::string request_;
	    bool useSandbox_;
	    const std::string& httpRequestHeader_;
	    std::string httpRequest_;
	    boost::asio::streambuf httpResponse_;

    };
}

#endif /* YDCLIENT_H */
