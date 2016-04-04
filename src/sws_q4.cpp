/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>

int main(int argc, char *argv[]) {
    int listenFd, connFd, numRead;
    time_t	rawTime;
    struct tm* timeInfo;
    struct sockaddr_in servAddr;
    char buffer[4096];

    if (argc < 2) {
        fprintf(stderr, "Usage: ./program [serverFolder]\n");
        exit(EXIT_FAILURE);
    }

    std::string rootDocument(argv[1]);

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

        std::string str(buffer);
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        std::vector<std::string> result;

        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            result.push_back(token);
            str.erase(0, pos + delimiter.length());
        }

        std::string filePath = rootDocument + result.at(1);

        if (!(filePath.substr(filePath.find_last_of(".") + 1) == "html")) {
            filePath.append("/index.html");
        }

        std::cout << "opening file :" << filePath << std::endl;
        std::stringstream payloadStream;
        FILE *file = fopen(filePath.c_str(), "r");

        if (file != NULL){
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                std::string line(buffer);
                std::cout << line << std::endl;
                payloadStream << line;
            }

            fclose(file);
        } else {
            std::cout << "file NOT found" << std::endl;
        }

        // HTML header
        std::string payload = payloadStream.str();
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        strftime(buffer, sizeof(buffer), "%c", timeInfo);
        std::stringstream headerResponse;
        headerResponse << "HTTP/1.1 200 OK\r" << std::endl
        << "Date: " << buffer << "\r" << std::endl
        << "Server: Sws\r" << std::endl
        << "Accept-Ranges: bytes\r" << std::endl
        << "Content-Length: "<< payload.length() << "\r" << std::endl
        << "Content-Type: text/html\r" << std::endl
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
