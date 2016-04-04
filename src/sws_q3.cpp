/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

int main() {
    int listenFd, connFd, numRead;
    time_t	rawTime;
    struct tm* timeInfo;
    struct sockaddr_in servAddr;
    char buffer[4096];

    // creating an endpoint for communication
    if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        exit(1);
    }

    // defining the characterics of the potential client and the port
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(8080);

    // binding to the port
    if (bind(listenFd, (struct sockaddr*) &servAddr, sizeof(servAddr))) {
        perror("bind error");
        exit(1);
    }

    // listening to a possible connection to the given port
    if (listen(listenFd, 1024) < 0) {
        perror("listen");
        exit(1);
    }

    // answering a connection
    for (;;) {
      // accepting a connection
        if ((connFd = accept(listenFd, (struct sockaddr *) NULL, NULL)) < 0) {
            perror("accept error");
            exit(1);
        }

        numRead = read(connFd, buffer, 4096);

        if (write(STDOUT_FILENO, buffer, numRead) != numRead) {
            perror("write error");
            exit(1);
        }

        // HTML header
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        strftime(buffer, sizeof(buffer), "%c", timeInfo);
        std::stringstream headerResponse;
        std::string payload;
        payload = "HTML page";
        headerResponse << "HTTP/1.1 200 OK\r" << std::endl
        << "Date: " << buffer << "\r" << std::endl
        << "Server: Sws\r" << std::endl
        << "Accept-Ranges: bytes\r" << std::endl
        << "Content-Length: " << payload.length() << "\r" << std::endl
        << "Content-Type: text/plain\r" << std::endl
        << std::endl
        << payload << "\r";

        std::string response = headerResponse.str();

        if (write(connFd, response.c_str(), response.size()) == -1) {
            perror("write error");
        }

        // closing the connection
        close(connFd);
        exit(0);
    }
}
