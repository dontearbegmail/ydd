.PHONY: all clean tests cdbo ct
CPPFLAGS = -g -Wall -std=c++11 -D_GNU_SOURCE -lboost_system -lssl -lcrypto -lpthread -lmysqlpp -lmysqlclient -I/usr/include/mysql/
SRCS = ydremote.cpp ydclient.cpp ydrequest.cpp ydrgetversion.cpp ydrcreatewsreport.cpp ydrgetwsreportlist.cpp ydrdeletewsreport.cpp deleteoldreports.cpp ydprocess.cpp yddconf.cpp dbconn.cpp ydbasetask.cpp
OBJS = $(SRCS:.cpp=.o)

TESTSCPPFLAGS = $(CPPFLAGS) -lboost_unit_test_framework
TESTS = $(wildcard tests/*.cpp)
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
	$(CXX) $(TESTSCPPFLAGS) -o tests/ydd-tests $(OBJS) $(TESTSOBJS);
	tests/ydd-tests

