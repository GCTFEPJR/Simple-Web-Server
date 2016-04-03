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

using namespace std;

void closeAll(int epfd, int clientFd, int timerFd) {
    if (clientFd != -1) {
        close(clientFd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, clientFd, NULL);
    }

    if (timerFd != -1) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, timerFd, NULL);
    }
}

void createClient(int listen_fd, int epfd, struct epoll_event ev, struct epoll_event timer_ev, std::map<int, int>* timerMap, itimerspec newValue) {
    int client_fd = accept(listen_fd, NULL, NULL);
    int timer_fd;

    if (client_fd == -1 && errno != EWOULDBLOCK) {
        exit(EXIT_FAILURE);
    }

    if (client_fd != -1) {
        printf("Accept a new connection...\n");
        int flags = fcntl(client_fd, F_GETFL);

        if (fcntl(client_fd, F_SETFL, flags) == -1) {
            close(client_fd);
        } else {
            ev.data.fd = client_fd;

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                close(client_fd);
            } else {
                int flags_tfd = TFD_NONBLOCK;

                if ((timer_fd = timerfd_create(CLOCK_REALTIME, flags_tfd)) < 0) {
                    perror("timer fd error");
                    closeAll(epfd, client_fd, -1);
                } else {
                    timer_ev.data.fd = timer_fd;
                    (*timerMap)[client_fd] = timer_fd;
                    (*timerMap)[timer_fd] = client_fd;

                    if (timerfd_settime(timer_fd, 0, &newValue, NULL) < 0) {
                        perror("time fd set time error");
                        closeAll(epfd, client_fd, -1);
                    } else {
                        if (epoll_ctl(epfd, EPOLL_CTL_ADD, timer_fd, &timer_ev) == -1) {
                            closeAll(epfd, client_fd, -1);
                        }
                    }
                }
            }
        }
    }
}

void handleClient(int epfd, int clientFd, int timerFd, itimerspec newValue, char* buffer, int bufferSize, std::string rootDocument) {
    if (timerfd_settime(timerFd, 0, &newValue, NULL) < 0) {
        perror("time fd set time reset error");
        closeAll(epfd, clientFd, timerFd);
    }

    int num_read = read(clientFd, buffer, bufferSize);

    if (write(STDOUT_FILENO, buffer, num_read) != num_read) {
        perror("write error");
        exit(1);
    }

    std::string response = handleRequest(buffer, rootDocument, bufferSize);

    if (write(clientFd, response.c_str(), response.size()) == -1) {
        closeAll(epfd, clientFd, timerFd);
    }
}

string handleRequest(const char* request, string rootDocument, int bufferSize){
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

    string filepath = rootDocument;

    if (result.size() > 0) {
        filepath += result.at(1);
    }

    if(!(filepath.substr(filepath.find_last_of(".") + 1) == "html")) {
        filepath.append("/index.html");
    }

    cout << "opening file :" << filepath << endl;

    stringstream payloadStream;
    FILE *f = fopen(filepath.c_str(),"r");

    char buff[bufferSize];

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

    return buildResponse(payload, responseCode, bufferSize);

}

string buildResponse(string payload, string responseCode, int bufferSize){

    char buffer[bufferSize];
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