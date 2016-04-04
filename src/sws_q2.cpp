/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int listenFd, connFd, numRead;
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
        if ((connFd = accept(listenFd, (struct sockaddr*) NULL, NULL)) < 0) {
            perror("accept errror");
            exit(1);
        }

        while ((numRead = read(connFd, buffer, 4096)) > 0){
            if (write(STDOUT_FILENO, buffer, numRead) != numRead) {
                perror("write error");
                exit(1);
            }
        }

        // closing the connection
        close(connFd);
    }
}
