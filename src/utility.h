#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <map>

void closeAll(int epfd, int clientFd, int timerFd);
void handleClient(int epfd, int clientFd, int timerFd, itimerspec newValue, char* buffer, int bufferSize, std::string rootDocument);
void createClient(int listen_fd, int epfd, struct epoll_event ev, struct epoll_event timer_ev, std::map<int, int>* timerMap, itimerspec newValue);
void errExit(const char *format, ...);
void fatal(const char *format, ...);
std::string handleRequest(const char* request, std::string rootDocument, int bufferSize);
std::string buildResponse(std::string payload,std::string responseCode, int bufferSize);

#endif
