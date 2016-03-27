.PHONY: all clean
CPPFLAGS = -Wall -std=c++11 -D_GNU_SOURCE -lboost_system -lssl -lcrypto -lpthread
SRCS = ydremote.cpp ydclient.cpp ydrequest.cpp ydrgetversion.cpp ydrcreatewsreport.cpp ydrgetwsreportlist.cpp
OBJS = $(SRCS:.cpp=.o)

all: ydd 

clean:
	rm -rf ydd *.o

ydd: ydd.cpp $(OBJS)
	$(CXX) $(CPPFLAGS) -o ydd ydd.cpp $(OBJS)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@
