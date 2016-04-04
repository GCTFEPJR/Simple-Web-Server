/* NETWORK PROGRAMMING: Jonathan Rozanes | Guillaume Catto | Thomas Fossati | Edouard Piette */

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <map>

#include "utility.h"
#include "inet_socket.h"

#define BUFFER_SIZE 32768
#define EPOLL_SIZE 20
#define MAX_EVENTS 10

int main(int argc, char *argv[]) {
    /* creating the listen file descriptor */
    int listenFd = inetListen("8080", 5, NULL);
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    struct itimerspec newValue;
    struct itimerspec getTime;
    struct timespec timeSpec;
    std::map<int,int> timerMap;
    int epFd;
    struct epoll_event ev; // event for clients
    struct epoll_event timerEv; // event for timers
    char buffer[BUFFER_SIZE];
    struct epoll_event evList[MAX_EVENTS];
    char x = 0;

    timeSpec.tv_sec = 5;
    timeSpec.tv_nsec = 0;
    newValue.it_value = timeSpec;

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
    timerEv.events = EPOLLIN;
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
        int nbFdReady = epoll_wait(epFd, evList, MAX_EVENTS, 100);

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
                if ((read(STDIN_FILENO, &x, 1) == 1) && (x == 'q'))
                    break;
            } else if (fd == listenFd) {
                /* new client */
                createClient(listenFd, epFd, ev, timerEv, &timerMap, newValue);
            } else if (timerfd_gettime(fd, &getTime) == 0) {
                /* timeout */
                closeAll(epFd, timerMap[fd], fd);
            } else {
                /* handling client */
                handleClient(epFd, fd, timerMap[fd], newValue, buffer, BUFFER_SIZE, rootDocument);
            }
        }
    }

    // closing the connection
    close(listenFd);

    return EXIT_SUCCESS;
}
