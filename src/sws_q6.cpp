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
#include <vector>
#include "inet_socket.c"

#define BUFFER_SIZE 4096
#define EPOLL_SIZE 10
#define MAX_EVENTS 10

using namespace std;

int main() {
    time_t	rawtime;
    struct tm* timeinfo;
    int listen_fd = inetListen("8080", 5, NULL);
    int flags = fcntl(STDIN_FILENO, F_GETFL);

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
        int nb_fd_ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);

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

                if (client_fd == -1 && errno != EWOULDBLOCK) {
                    exit(EXIT_FAILURE);
                }

                if (client_fd != -1) {
                    printf("Accept a new connection...\n");
                    flags = fcntl(client_fd, F_GETFL);
                    if (fcntl(client_fd, F_SETFL, flags|O_NONBLOCK) == -1) {
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

                string str(buffer);
                string delimiter = " ";


                size_t pos = 0;
                std::string token;
                vector<string> result;

                while ((pos = str.find(delimiter)) != std::string::npos) {
                    token = str.substr(0, pos);
                    result.push_back(token);
                    str.erase(0, pos + delimiter.length());
                }

                string filepath = "/srv/www/SimpleWebServer" + result.at(1);


                if(!(filepath.substr(filepath.find_last_of(".") + 1) == "html")) {
                    filepath.append("/index.html");
                }

                cout << "opening file :" << filepath << endl;

                stringstream payloadStream;
                FILE *f = fopen(filepath.c_str(),"r");

                char buff[50];

                std::stringstream headerResponse;

                if( f != NULL){
                    cout << "file found" << endl;
                    while(fgets(buff, sizeof(buff),f) != NULL){
                        string line(buff);
                        cout << line << endl;
                        payloadStream << line;

                    }
                    fclose(f);
                    headerResponse << "HTTP/1.1 200 OK\r" << std::endl;

                } else{
                    cout << "file NOT found" << endl;
                    payloadStream << "<html><body><h1><center>404 Not Found</center></h1></body></html>";
                    headerResponse << "HTTP/1.1 404 Not Found\r" << std::endl;
                }


                string payload = payloadStream.str();
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(buffer, sizeof(buffer), "%c", timeinfo);

                headerResponse << "Date: " << buffer << "\r" << std::endl
                << "Server: Sws\r" << std::endl
                << "Accept-Ranges: bytes\r" << std::endl
                << "Content-Length: "<<payload.length()<<"\r" << std::endl
                << "Content-Type: text/html\r" << std::endl
                << std::endl
                << payload << "\r";/**/

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