# NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette

CC=gcc
CXX=g++
CPPFLAGS=-std=c++11
RM=rm -f

SRCS=inet_socket.cpp utility.cpp
OBJS=inet_socket.o utility.o

sws_q7: $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o sws_q7 sws_q7.cpp

sws_q6: $(OBJS)
	$(CXX) $(CPPFLAGS) $(OBJS) -o sws_q6 sws_q6.cpp

sws_q5: $(OBJS)
	$(CXX) $(CPPFLAGS)  $(OBJS) -o sws_q5 sws_q5.cpp

sws_q4: $(OBJS)
	$(CXX) $(CPPFLAGS) -o sws_q4 sws_q4.cpp

sws_q3: $(OBJS)
	$(CXX) $(CPPFLAGS) -o sws_q3 sws_q3.cpp

sws_q2: $(OBJS)
	$(CXX) $(CPPFLAGS) -o sws_q2 sws_q2.cpp

all: sws_q7 sws_q6 sws_q5 sws_q4 sws_q3 sws_q2

object: $(SRCS)
	$(CXX) $(CPPFLAGS) -c $(SRCS)

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) sws_q2 sws_q3 sws_q4 sws_q5 sws_q6 sws_q7
