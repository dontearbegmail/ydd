#include "../dbconn.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(construct_DbConn)
{
    ydd::DbConn dbconn;
    BOOST_REQUIRE(dbconn.getCurrentUserId() == 0);
    BOOST_REQUIRE(dbconn.getCurrentSchema().empty());
}

BOOST_AUTO_TEST_CASE(switchUser)
{
    ydd::DbConn dbconn;
    BOOST_REQUIRE_THROW(dbconn.switchUser(110), sql::SQLException);
    BOOST_REQUIRE(dbconn.getCurrentUserId() == 110);
    BOOST_REQUIRE(dbconn.getCurrentSchema() == "ydd_110");

    BOOST_REQUIRE_NO_THROW(dbconn.switchUser(110));
    BOOST_REQUIRE(dbconn.getCurrentUserId() == 110);
    BOOST_REQUIRE(dbconn.getCurrentSchema() == "ydd_110");

    BOOST_WARN_THROW(dbconn.switchUser(173025), sql::SQLException);
    BOOST_REQUIRE(dbconn.getCurrentUserId() == 173025);
    BOOST_REQUIRE(dbconn.getCurrentSchema() == "ydd_173025");

}
