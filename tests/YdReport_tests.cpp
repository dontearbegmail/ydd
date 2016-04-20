#include "../ydphrase.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <vector>

using namespace ydd;

BOOST_AUTO_TEST_CASE(YdPhrase_constructor)
{
    std::vector<YdReport> v;
    YdReport t;
    BOOST_REQUIRE_EQUAL(t.state, ydd::GeneralState::init);
    BOOST_REQUIRE_EQUAL(t.isFinished, false);

    v.push_back({{}, true});
    v.push_back({});
    v.push_back({{}, false});
    v.push_back({{}, false, ydd::GeneralState::yderrBadAuth});
    BOOST_REQUIRE_EQUAL(v[0].state, ydd::GeneralState::init);
    BOOST_REQUIRE_EQUAL(v[0].isFinished, true);
    BOOST_REQUIRE_EQUAL(v[1].state, ydd::GeneralState::init);
    BOOST_REQUIRE_EQUAL(v[1].isFinished, false);
    BOOST_REQUIRE_EQUAL(v[2].state, ydd::GeneralState::init);
    BOOST_REQUIRE_EQUAL(v[2].isFinished, false);
    BOOST_REQUIRE_EQUAL(v[3].state, ydd::GeneralState::yderrBadAuth);
    BOOST_REQUIRE_EQUAL(v[3].isFinished, false);
}
