#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string>
#include <sys/timerfd.h>
#include <map>

#include "utility.cpp"
#include "inet_socket.c"

#define BUFFER_SIZE 32768
#define EPOLL_SIZE 20
#define MAX_EVENTS 10

using namespace std;

int main(int argc, char *argv[]) {
    time_t	rawtime;
    int listen_fd = inetListen("8080", 5, NULL);
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    struct itimerspec newValue;
    struct itimerspec getTime;
    struct timespec timespec;
    std::map<int,int> timerMap;
    timespec.tv_sec = 5;
    timespec.tv_nsec = 0;
    newValue.it_value = timespec;

    if (argc < 2) {
        fprintf(stderr, "usage: ./SimpleWebServer [serverFolder]\n");
        exit(EXIT_FAILURE);
    }

    string rootDocument(argv[1]);

    if (listen_fd == -1) {
        exit(EXIT_FAILURE);
    }

    if (fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK) == -1) {
        exit(EXIT_FAILURE);
    }

    flags = fcntl(listen_fd, F_GETFL);

    if (fcntl(listen_fd, F_SETFL, flags|O_NONBLOCK) == -1) {
        exit(EXIT_FAILURE);
    }

    int epfd;
    epfd = epoll_create(EPOLL_SIZE);

    if (epfd == -1) {
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    struct epoll_event timer_ev;

    ev.events = EPOLLIN;
    timer_ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        exit(EXIT_FAILURE);
    }

    ev.data.fd = STDIN_FILENO;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    struct epoll_event evlist[MAX_EVENTS];
    char x = 0;

    while (x != 'q') {
        time(&rawtime);
        rawtime = time(NULL);

        int nb_fd_ready = epoll_wait(epfd, evlist, MAX_EVENTS, 100);

        if (nb_fd_ready == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0 ; i < nb_fd_ready ; ++i) {
            int fd = evlist[i].data.fd;

            if (fd == STDIN_FILENO) {
                if ((read(STDIN_FILENO, &x, 1) == 1) && (x == 'q'))
                    break;
            } else if (fd == listen_fd) {
                createClient(listen_fd, epfd, ev, timer_ev, &timerMap, newValue);
            } else if (timerfd_gettime(fd, &getTime) == 0) {
                closeAll(epfd, timerMap[fd], fd);
            } else {
                handleClient(epfd, fd, timerMap[fd], newValue, buffer, BUFFER_SIZE, rootDocument);
            }
        }
    }

    close(listen_fd);

    return EXIT_SUCCESS;
}