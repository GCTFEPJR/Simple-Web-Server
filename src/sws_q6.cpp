/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/epoll.h>

#include "inet_socket.h"
#include "utility.h"

#define BUFFER_SIZE 4096
#define EPOLL_SIZE 10
#define MAX_EVENTS 10

int main(int argc, char *argv[]) {
    time_t	rawTime;
    struct tm* timeInfo;
    /* creating the listen file descriptor */
    int listenFd = inetListen("8080", 5, NULL);
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    int epFd;
    char buffer[BUFFER_SIZE];
    struct epoll_event evList[MAX_EVENTS];
    char x = 0;
    struct epoll_event ev;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./program [serverFolder]\n");
        exit(EXIT_FAILURE);
    }

    std::string rootDocument(argv[1]);

    if (listenFd == -1) {
        exit(EXIT_FAILURE);
    }

    if (fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK) == -1) {
        exit(EXIT_FAILURE);
    }

    flags = fcntl(listenFd, F_GETFL);

    if (fcntl(listenFd, F_SETFL, flags|O_NONBLOCK) == -1) {
        exit(EXIT_FAILURE);
    }

    // epoll creation
    epFd = epoll_create(EPOLL_SIZE);

    if (epFd == -1) {
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenFd; // adding the listen file descriptor

    if (epoll_ctl(epFd, EPOLL_CTL_ADD, listenFd, &ev) == -1) {
        exit(EXIT_FAILURE);
    }

    ev.data.fd = STDIN_FILENO; // adding the STDIN_FILENO file descriptor

    if (epoll_ctl(epFd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        exit(EXIT_FAILURE);
    }

    /* stopping if 'q' has been pressed */
    while (x != 'q') {
        int nbFdReady = epoll_wait(epFd, evList, MAX_EVENTS, -1);

        if (nbFdReady == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < nbFdReady; ++i) {
            int fd = evList[i].data.fd;

            if (fd == STDIN_FILENO) {
                if ((read(STDIN_FILENO, &x, 1) == 1) && (x == 'q')) {
                    break;
                }
            } else if (fd == listenFd) {
                /* new client */
                int clientFd = accept(listenFd, NULL, NULL);

                if (clientFd == -1 && errno != EWOULDBLOCK) {
                    exit(EXIT_FAILURE);
                }

                if (clientFd != -1) {
                    std::cout << "Accept a new connection..." << std::endl;

                    flags = fcntl(clientFd, F_GETFL);

                    if (fcntl(clientFd, F_SETFL, flags|O_NONBLOCK) == -1) {
                        close(clientFd);
                    } else {
                        ev.data.fd = clientFd;
                        if (epoll_ctl(epFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
                            close(clientFd);
                        }
                    }
                }
            } else {
                /* handling client */
                int numRead = read(fd, buffer, 4096);

                if (write(STDOUT_FILENO, buffer, numRead) != numRead) {
                    perror("write error");
                    exit(1);
                }

                std::string response = handleRequest(buffer, rootDocument,
                  BUFFER_SIZE);

                if (write(fd, response.c_str(), response.size()) == -1) {
                    close(fd);
                }
            }
        }
    }

    /* closing connection */
    close(listenFd);

    return EXIT_SUCCESS;
}
