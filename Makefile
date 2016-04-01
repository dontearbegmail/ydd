.PHONY: all clean tests cdbo ct
CPPFLAGS = -g -Wall -std=c++11 -D_GNU_SOURCE -lboost_system -lssl -lcrypto -lpthread -lmysqlcppconn
SRCS = ydremote.cpp ydclient.cpp ydrequest.cpp ydrgetversion.cpp ydrcreatewsreport.cpp ydrgetwsreportlist.cpp ydrdeletewsreport.cpp deleteoldreports.cpp ydprocess.cpp ydprocesslog.cpp dbconn.cpp yddconf.cpp
OBJS = $(SRCS:.cpp=.o)

TESTSCPPFLAGS = $(CPPFLAGS) -lboost_unit_test_framework
TESTS = tests/DbConn_test.cpp
TESTSOBJS = $(TESTS:.cpp=.o)

all: ydd tests

clean:
	rm -rf ydd *.o

cdbo:
	rm -rf dbconn.o

ydd: main.cpp $(OBJS)
	$(CXX) $(CPPFLAGS) -o ydd main.cpp $(OBJS)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@


ct:
	rm -rf tests/tests tests/*.o

tests/%.o : tests/%.cpp
	$(CXX) $(TESTSCPPFLAGS) -c -o $@ $<

tests: $(TESTSOBJS)
	$(CXX) $(TESTSCPPFLAGS) -o tests/tests tests/main.cpp $(OBJS) $(TESTSOBJS);
	tests/tests

