#ifndef UTILITY_H
#define UTILITY_H

#include <string>

void errExit(const char *format, ...);

/* Print an error message (without an 'errno' diagnostic) */
void fatal(const char *format, ...);


std::string handleRequest(const char* request, std::string rootDocument);
std::string buildResponse(std::string payload,std::string responseCode);

#endif
