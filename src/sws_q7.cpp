#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <complex>

#include "inet_socket.c"

#define BUFFER_SIZE 4096
#define EPOLL_SIZE 10
#define MAX_EVENTS 10
#define TIMEOUT 5

typedef struct fdTimer {
    int fd;
    time_t time;
} fdTimer;

bool testDiff(struct fdTimer fdt) {
    time_t rawtime;
    time(&rawtime);

    return (std::fabs((long long) difftime(fdt.time, rawtime))) > TIMEOUT;
}

int main() {
    time_t	rawtime;
    struct tm* timeinfo;
    int listen_fd = inetListen("8080", 5, NULL);
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    int currentfdTimerSize = 0;
    std::list<fdTimer>* fds = new std::list<fdTimer>();

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
    ev.events = EPOLLIN;
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

        for (std::list<fdTimer>::iterator it=fds->begin(); it != fds->end(); ++it) {
            if ((std::fabs((long long) difftime((*it).time, rawtime))) > TIMEOUT) {
                close((*it).fd);
                std::string info = "CLOSING FD AFTER TIMEOUT.";
                std::cout << info << std::endl;
                epoll_ctl(epfd, EPOLL_CTL_DEL, (*it).fd, NULL);

            }
        }

        fds->remove_if(testDiff);
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
                int client_fd = accept(listen_fd, NULL, NULL);

                // Adding the local time to the fd
                fdTimer newTimer;
                newTimer.fd = client_fd;
                time(&newTimer.time);
                fds->push_back(newTimer);

                if (client_fd == -1 && errno != EWOULDBLOCK) {
                    exit(EXIT_FAILURE);
                }

                if (client_fd != -1) {
                    printf("Accept a new connection...\n");
                    flags = fcntl(client_fd, F_GETFL);
                    if (fcntl(client_fd, F_SETFL, flags) == -1) {
                        close(client_fd);
                    } else {
                        ev.data.fd = client_fd;
                        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                            close(client_fd);
                        }
                    }
                }
            } else {
                int num_read;
                num_read = read(fd, buffer, 4096);
                if (write(STDOUT_FILENO, buffer, num_read) != num_read) {
                    perror("write error");
                    exit(1);
                }

                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(buffer, sizeof(buffer), "%c", timeinfo);
                std::stringstream headerResponse;
                std::string payload;
                payload = "<html><body><h1>Test HTML Page</h1></body></html>";
                headerResponse << "HTTP/1.1 200 OK\r" << std::endl
                << "Date: " << buffer << "\r" << std::endl
                << "Server: Sws\r" << std::endl
                << "Accept-Ranges: bytes\r" << std::endl
                << "Content-Length: "<<payload.length()<<"\r" << std::endl
                << "Content-Type: text/html\r" << std::endl
                << std::endl
                << payload << "\r";

                std::string response = headerResponse.str();

                if (write(fd, response.c_str(), response.size()) == -1) {
                    close(fd);
                }
            }
        }
    }

    close(listen_fd);

    return EXIT_SUCCESS;
}