#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utility.h"

#define BUFFER_SIZE 4096

using namespace std;

int main(int argc, char *argv[]) {
    int listenfd,connfd;
    int numRead;
    time_t	rawtime;
    struct tm* timeinfo;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    if (argc < 2) {
        fprintf(stderr, "usage: ./SimpleWebServer [serverFolder]\n");
        exit(EXIT_FAILURE);
    }

    string rootDocument(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        exit(1);
    }
    // Define the characterics of the potential client and the port
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    // Bind to the port
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) {
        perror("bind error");
        exit(1);
    }

    // Listen to a possible connnexion to the given port
    if (listen(listenfd, 1024) < 0) {
        perror("listen");
        exit(1);
    }

    // Answer the connection
    for (;;) {
        // Accept the connection
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) {
            perror("accept error");
            exit(1);
        }
        /*while ((*/numRead = read(connfd, buffer, sizeof(buffer)+1);/*) > 0) {*/
        if (write(STDOUT_FILENO, buffer, numRead) != numRead) {
            perror("write error");
            exit(1);
        }


        string response = handleRequest(buffer, rootDocument);


        if (write(connfd, response.c_str(), response.size()) == -1){
            printf("pb");
        }else {
            printf("ok");
        }

        // Close the connection
        close(connfd);

        exit(0);
    }


}
