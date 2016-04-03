#include "../ydrequest.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace ydd;

class TestYdRequest : public YdRequest
{
    public:
    TestYdRequest(std::string& token, boost::asio::io_service& ios, bool useSandbox,
	    YdProcess::Callback ydProcessCallback) :
	YdRequest(token, ios, useSandbox, ydProcessCallback)
    {
    }

    void testInit_emptyPtRequest()
    {
	BOOST_REQUIRE_EQUAL(state_, init);
	doInit();
	/* because of empty ptRequest_ */
	BOOST_REQUIRE_EQUAL(state_, initError);
	BOOST_REQUIRE_EQUAL(request_, "");
    }

    void testInit_ptRequest1()
    {
	BOOST_REQUIRE_EQUAL(state_, init);
	ptRequest_.put("method", "MyMethod");
	doInit();
	BOOST_REQUIRE_EQUAL(state_, initOk);
	BOOST_REQUIRE_EQUAL(request_, "{\"method\":\"MyMethod\"}\n");
    }

    void testInit_ptRequestWithQuotes()
    {
	BOOST_REQUIRE_EQUAL(state_, init);
	ptRequest_.put("data", "\"гипсокартон монтаж\"");
	doInit();
	BOOST_REQUIRE_EQUAL(state_, initOk);
	BOOST_REQUIRE_EQUAL(request_, "{\"data\":\"\\\"гипсокартон монтаж\\\"\"}\n");
    }

    void testInit_ptRequestWithOneQuote()
    {
	// !{]!@#$%^&*()_-=±~/?<>,.;
	BOOST_REQUIRE_EQUAL(state_, init);
	ptRequest_.put("data", "\"настоящий запрос");
	doInit();
	BOOST_REQUIRE_EQUAL(state_, initOk);
	BOOST_REQUIRE_EQUAL(request_, "{\"data\":\"\\\"настоящий запрос\"}\n");
    }

    void testInit_ptRequestWithSpecialChars()
    {
	BOOST_REQUIRE_EQUAL(state_, init);
	ptRequest_.put("data", "!{]}]@#$%^&*()_/-=±~?\\<>,.;");
	doInit();
	BOOST_REQUIRE_EQUAL(state_, initOk);
	BOOST_REQUIRE_EQUAL(request_, "{\"data\":\"!{]}]@#$%^&*()_\\/-=±~?\\\\<>,.;\"}\n");
    }

    void testConstructor(std::string& token, boost::asio::io_service& ios, bool useSandbox, 
	    YdProcess::Callback callback)
    {
	BOOST_REQUIRE_EQUAL(token_, token);
	BOOST_REQUIRE_EQUAL(getState(), init);
	BOOST_REQUIRE_EQUAL(useSandbox_, useSandbox);
	BOOST_REQUIRE_EQUAL(get(), "");
	BOOST_REQUIRE_EQUAL(getJsonResponse(), "");
	BOOST_REQUIRE_EQUAL(ydError_.error_code, 0);
	BOOST_REQUIRE_EQUAL(ydError_.error_detail, "");
	BOOST_REQUIRE_EQUAL(ydError_.error_str, "");
	std::string errStr;
	getYdErrorString(errStr);
	BOOST_REQUIRE_EQUAL(errStr, "YDAPI error: error_code = 0, error_str = \"\", "
		"error_detail = \"\"");
    }
};

struct FxYdRequest
{
    FxYdRequest() :
	ios(),
	token("123"),
	useSandbox(true),
	tydr(token, ios, useSandbox, NULL)
    {
    }
    boost::asio::io_service ios;
    std::string token;
    bool useSandbox;
    TestYdRequest tydr;
};


BOOST_FIXTURE_TEST_CASE(constructor, FxYdRequest)
{
    tydr.testConstructor(token, ios, useSandbox, NULL);
}

BOOST_FIXTURE_TEST_CASE(getYdErrorString, FxYdRequest)
{
    const YdRequest::YdError& e = tydr.getYdError();
    YdRequest::YdError* pe = const_cast<YdRequest::YdError*>(&e);
    pe->error_code = 31415;
    pe->error_str = "Invalid login";
    pe->error_detail = "How detailed can it be?";
    std::string es;
    tydr.getYdErrorString(es);
    BOOST_REQUIRE_EQUAL(es, "YDAPI error: error_code = 31415, error_str = "
	    "\"Invalid login\", error_detail = \"How detailed can it be?\"");

    pe->error_code = 173000;
    pe->error_str = "Don't know what's the sence of doing it";
    pe->error_detail = "But I still do it";
    tydr.getYdErrorString(es);
    BOOST_REQUIRE_EQUAL(es, "YDAPI error: error_code = 173000, error_str = "
	    "\"Don't know what's the sence of doing it\", error_detail = "
	    "\"But I still do it\"");
}

BOOST_FIXTURE_TEST_CASE(init_emptyPtRequest, FxYdRequest)
{
    tydr.testInit_emptyPtRequest();
}

BOOST_FIXTURE_TEST_CASE(init_ptRequest1, FxYdRequest)
{
    tydr.testInit_ptRequest1();
}

BOOST_FIXTURE_TEST_CASE(init_ptRequestWithQuotes, FxYdRequest)
{
    tydr.testInit_ptRequestWithQuotes();
}

BOOST_FIXTURE_TEST_CASE(init_ptRequestWithSpecialChars, FxYdRequest)
{
    tydr.testInit_ptRequestWithSpecialChars();
}

BOOST_FIXTURE_TEST_CASE(init_ptRequestWithOneQuote, FxYdRequest)
{
    tydr.testInit_ptRequestWithOneQuote();
}
