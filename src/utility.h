/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <map>

void closeAll(int epFd, int clientFd, int timerFd);
void handleClient(int epFd, int clientFd, int timerFd, itimerspec newValue, char* buffer, int bufferSize, std::string rootDocument);
void createClient(int listenFd, int epFd, struct epoll_event ev, struct epoll_event timerEv, std::map<int, int>* timerMap, itimerspec newValue);
std::string handleRequest(const char* request, std::string rootDocument, int bufferSize);
std::string buildResponse(std::string payload,std::string responseCode, int bufferSize);

#endif
