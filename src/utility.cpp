/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include "utility.h"

/* closing the client connection and its given timer */
void closeAll(int epFd, int clientFd, int timerFd) {
    if (clientFd != -1) {
        close(clientFd);
        epoll_ctl(epFd, EPOLL_CTL_DEL, clientFd, NULL);
    }

    if (timerFd != -1) {
        epoll_ctl(epFd, EPOLL_CTL_DEL, timerFd, NULL);
    }
}

/* creating the client connection and tis given timer */
void createClient(int listenFd, int epFd, struct epoll_event ev,
  struct epoll_event timerEv, std::map<int, int>* timerMap, itimerspec newValue) {
    int clientFd = accept(listenFd, NULL, NULL);
    int timerFd;

    if (clientFd == -1 && errno != EWOULDBLOCK) {
        exit(EXIT_FAILURE);
    }

    if (clientFd != -1) {
        std::cout << "Accept a new connection..." << std::endl;

        int flags = fcntl(clientFd, F_GETFL);

        if (fcntl(clientFd, F_SETFL, flags) == -1) {
            close(clientFd);
        } else {
            /* adding the client */
            ev.data.fd = clientFd;

            if (epoll_ctl(epFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
                close(clientFd);
            } else {
                int flags_tfd = TFD_NONBLOCK;

                if ((timerFd = timerfd_create(CLOCK_REALTIME, flags_tfd)) < 0) {
                    perror("timer fd error");
                    closeAll(epFd, clientFd, -1);
                } else {
                    timerEv.data.fd = timerFd;
                    (*timerMap)[clientFd] = timerFd;
                    (*timerMap)[timerFd] = clientFd;

                    if (timerfd_settime(timerFd, 0, &newValue, NULL) < 0) {
                        perror("time fd set time error");
                        closeAll(epFd, clientFd, -1);
                    } else {
                        /* adding the timer */
                        if (epoll_ctl(epFd, EPOLL_CTL_ADD, timerFd, &timerEv) == -1) {
                            closeAll(epFd, clientFd, -1);
                        }
                    }
                }
            }
        }
    }
}

void handleClient(int epFd, int clientFd, int timerFd, itimerspec newValue,
  char* buffer, int bufferSize, std::string rootDocument) {
    if (timerfd_settime(timerFd, 0, &newValue, NULL) < 0) {
        perror("time fd set time reset error");
        closeAll(epFd, clientFd, timerFd);
    }

    int numRead = read(clientFd, buffer, bufferSize);

    if (write(STDOUT_FILENO, buffer, numRead) != numRead) {
        perror("write error");
        exit(1);
    }

    std::string response = handleRequest(buffer, rootDocument, bufferSize);

    if (write(clientFd, response.c_str(), response.size()) == -1) {
        closeAll(epFd, clientFd, timerFd);
    }
}

/* handling a request */
std::string handleRequest(const char* request, std::string rootDocument,
  int bufferSize){
    std::string str(request);
    std::string delimiter = " ";

    size_t pos = 0;
    std::string token;
    std::vector<std::string> result;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + delimiter.length());
    }

    std::string filePath = rootDocument;

    if (result.size() > 0) {
        filePath += result.at(1);
    }

    if (!(filePath.substr(filePath.find_last_of(".") + 1) == "html")) {
        filePath.append("/index.html");
    }

    std::cout << "opening file :" << filePath << std::endl;

    std::stringstream payloadStream;
    FILE *file = fopen(filePath.c_str(),"r");

    char buffer[bufferSize];

    std::string responseCode;

    if (file != NULL){
        std::cout << "file found" << std::endl;

        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            std::string line(buffer);
            std::cout << line << std::endl;
            payloadStream << line;
        }

        fclose(file);
        responseCode = "HTTP/1.1 200 OK\r";
    } else {
        std::cout << "file NOT found" << std::endl;
        payloadStream <<
          "<html><body><h1><center>404 Not Found</center></h1></body></html>";
        responseCode = "HTTP/1.1 404 Not Found\r";
    }

    std::string payload = payloadStream.str();

    return buildResponse(payload, responseCode, bufferSize);
}

/* building the response for the client */
std::string buildResponse(std::string payload, std::string responseCode, int bufferSize) {
    char buffer[bufferSize];
    time_t	rawTime;
    struct tm* timeInfo;
    std::stringstream headerResponse;

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(buffer, sizeof(buffer), "%c", timeInfo);

    /* HTTP header */
    headerResponse << responseCode << std::endl;
    headerResponse << "Date: " << buffer << "\r" << std::endl
    << "Server: Sws\r" << std::endl
    << "Accept-Ranges: bytes\r" << std::endl
    << "Content-Length: " << payload.length() << "\r" << std::endl
    << "Content-Type: text/html\r" << std::endl
    << std::endl
    << payload << "\r";

    return headerResponse.str();
}
