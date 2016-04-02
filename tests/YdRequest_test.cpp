#include "../ydrequest.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(construct_YdRequest)
{
    using namespace ydd;
    boost::asio::io_service ios;
    std::string token = "asdf";
    YdRequest r(token, ios, true, NULL);
    BOOST_REQUIRE_EQUAL(r.getToken(), token);
    BOOST_REQUIRE_EQUAL(r.getState(), YdRequest::init);
    BOOST_REQUIRE_EQUAL(r.getUseSandbox(), true);
    BOOST_REQUIRE_EQUAL(r.get(), "");
    BOOST_REQUIRE_EQUAL(r.getJsonResponse(), "");
    const YdRequest::YdError& e = r.getYdError();
    BOOST_REQUIRE_EQUAL(e.error_code, 0);
    BOOST_REQUIRE_EQUAL(e.error_detail, "");
    BOOST_REQUIRE_EQUAL(e.error_str, "");
    std::string errStr;
    r.getYdErrorString(errStr);
    BOOST_REQUIRE_EQUAL(errStr, "YDAPI error: error_code = 0, error_str = \"\", "
	    "error_detail = \"\"");
}

BOOST_AUTO_TEST_CASE(getYdErrorString)
{
    using namespace ydd;
    boost::asio::io_service ios;
    std::string token = "asdf";
    YdRequest r(token, ios, true, NULL);

    const YdRequest::YdError& e = r.getYdError();
    YdRequest::YdError* pe = const_cast<YdRequest::YdError*>(&e);
    pe->error_code = 31415;
    pe->error_str = "Invalid login";
    pe->error_detail = "How detailed can it be?";
    std::string es;
    r.getYdErrorString(es);
    BOOST_REQUIRE_EQUAL(es, "YDAPI error: error_code = 31415, error_str = "
	    "\"Invalid login\", error_detail = \"How detailed can it be?\"");

    pe->error_code = 173000;
    pe->error_str = "Don't know what's the sence of doing it";
    pe->error_detail = "But I still do it";
    r.getYdErrorString(es);
    BOOST_REQUIRE_EQUAL(es, "YDAPI error: error_code = 173000, error_str = "
	    "\"Don't know what's the sence of doing it\", error_detail = "
	    "\"But I still do it\"");
}
