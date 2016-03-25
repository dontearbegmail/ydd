#include "ydremote.h"

using namespace boost::asio;

namespace ydd
{
    const std::string YdRemote::host = "api.direct.yandex.ru";
    const std::string YdRemote::hostSandbox = "api-sandbox.direct.yandex.ru";
    const std::string YdRemote::port = "443";
    const std::string YdRemote::httpHeader = 
	"POST /v4/json/ HTTP/1.1\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Host: " + YdRemote::host + "\r\n"
	"Content-Length: ";
    const std::string YdRemote::httpHeaderSandbox = 
	"POST /v4/json/ HTTP/1.1\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Host: " + YdRemote::hostSandbox + "\r\n"
	"Content-Length: ";
    ssl::context YdRemote::ctx = YdRemote::initCtx();
    ip::tcp::resolver::iterator YdRemote::hostIt = YdRemote::initHostIt(false);
    ip::tcp::resolver::iterator YdRemote::hostSandboxIt = YdRemote::initHostIt(true);

    ssl::context YdRemote::initCtx()
    {
	return ssl::context(ssl::context::sslv23);
    }

    ip::tcp::resolver::iterator YdRemote::initHostIt(bool useSandbox)
    {
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(
		useSandbox ? YdRemote::hostSandbox : YdRemote::host, YdRemote::port);
	return resolver.resolve(query);
    }

}
