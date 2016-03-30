#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <vector>

using namespace std;

int main() {
    int listenfd,connfd;
    int numRead;
    time_t	rawtime;
    struct tm* timeinfo;
    char buffer[40];
    struct sockaddr_in servaddr;
    char buff[4096];
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
        /*while ((*/numRead = read(connfd, buff, 4096);/*) > 0) {*/
        if (write(STDOUT_FILENO, buff, numRead) != numRead) {
            perror("write error");
            exit(1);
        }
        //}


        string str(buff);
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
