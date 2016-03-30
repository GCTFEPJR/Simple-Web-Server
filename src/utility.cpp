//
// Created by thomas on 30/03/16.
//
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <complex>
#include <vector>
#include "utility.h"

#define BUFFER_SIZE 4096

using namespace std;

string handleRequest(const char* request, string rootDocument){

    string str(request);
    string delimiter = " ";


    size_t pos = 0;
    std::string token;
    vector<string> result;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + delimiter.length());
    }

    string filepath = rootDocument + result.at(1);


    if(!(filepath.substr(filepath.find_last_of(".") + 1) == "html")) {
        filepath.append("/index.html");
    }

    cout << "opening file :" << filepath << endl;

    stringstream payloadStream;
    FILE *f = fopen(filepath.c_str(),"r");

    char buff[BUFFER_SIZE];

    string responseCode;

    if( f != NULL){
        cout << "file found" << endl;
        while(fgets(buff, sizeof(buff),f) != NULL){
            string line(buff);
            cout << line << endl;
            payloadStream << line;

        }
        fclose(f);
        responseCode = "HTTP/1.1 200 OK\r";

    } else{
        cout << "file NOT found" << endl;
        payloadStream << "<html><body><h1><center>404 Not Found</center></h1></body></html>";
        responseCode = "HTTP/1.1 404 Not Found\r";
    }

    string payload = payloadStream.str();

    return buildResponse(payload, responseCode);

}

string buildResponse(string payload, string responseCode){

    char buffer[BUFFER_SIZE];
    time_t	rawtime;
    struct tm* timeinfo;
    std::stringstream headerResponse;

    time(&rawtime);


    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%c", timeinfo);

    headerResponse << responseCode << std::endl;
    headerResponse << "Date: " << buffer << "\r" << std::endl
    << "Server: Sws\r" << std::endl
    << "Accept-Ranges: bytes\r" << std::endl
    << "Content-Length: "<<payload.length()<<"\r" << std::endl
    << "Content-Type: text/html\r" << std::endl
    << std::endl
    << payload << "\r";/**/

    return headerResponse.str();


}