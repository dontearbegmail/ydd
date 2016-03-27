#ifndef YDREMOTE_H
#define YDREMOTE_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

namespace ydd
{
    class YdRemote
    {
	public:
	    typedef unsigned long ReportIdType;
	    static const std::string host;
	    static const std::string hostSandbox;
	    static const std::string port;
	    static boost::asio::ssl::context ctx;
	    static boost::asio::ip::tcp::resolver::iterator hostIt;
	    static boost::asio::ip::tcp::resolver::iterator hostSandboxIt;
	    static const std::string httpHeader;
	    static const std::string httpHeaderSandbox;

	private:
	    static boost::asio::ssl::context initCtx();
	    static boost::asio::ip::tcp::resolver::iterator initHostIt(bool useSandbox);
    };
}

#endif // YDREMOTE_H
