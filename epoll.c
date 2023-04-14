#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/epoll.h>

#define BACKLOG      5
#define BUFF_SIZE    200
#define DEFAULT_PORT 6666
#define MAX_EVENTS   10

int main(int argc, char* argv[])
{
    int SERVER_PORT = DEFAULT_PORT;

    if (argc > 2) {
        printf("param err:\nUsage:\n\t%s port | %s\n\n", argv[0], argv[0]);
        return -1;
    }

    if (argc == 2) {
        SERVER_PORT = atoi(argv[1]);
    }

    printf("Listen Port: %d\nListening ...\n", SERVER_PORT);

    int nbytes = 0;
    int cliSocket = 0;
    int servSocket = 0;
    char buffer[BUFF_SIZE] = { 0 };
    socklen_t addrLen = 0;
    struct sockaddr_in cliAddr = { 0 };
    struct sockaddr_in servAddr = { 0 };

    struct epoll_event ev, readyEvents[MAX_EVENTS];
    int nfds, epollfd;

    if ((servSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket err\n");
        return -1;
    }

    int optval = 1;
    if (setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval))
        < 0) {
        printf("setsockopt error\n");
        return -1;
    }

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_port        = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(servSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("bind error\n");
        return -1;
    }

    if (listen(servSocket, BACKLOG) < 0) {
        printf("listen error\n");
        return -1;
    }

    if ((epollfd = epoll_create1(0)) == -1) {
        printf("epoll_create error\n");
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = servSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, servSocket, &ev) == -1) {
        printf("epoll_ctl error\n");
        return -1;
    }

    for (;;) {
        int timeout_ms = 2000;

        // check all epollfd poll in event
        if ((nfds = epoll_wait(epollfd, readyEvents, MAX_EVENTS, timeout_ms))
            == -1) {
            printf("epoll_wait error\n");
            return -1;
        }

        if (nfds == 0) {
            printf("epoll_wait timeout\n");
            continue;
        }

        for (int n = 0; n < nfds; n++) {
            // check a new accept socket fd
            if (readyEvents[n].data.fd == servSocket) {
                cliSocket
                    = accept(servSocket, (struct sockaddr*)&cliAddr, &addrLen);
                if (cliSocket == -1) {
                    printf("accept error\n");
                    return -1;
                }

                printf("New client connections client[%d] %s:%d\n", cliSocket,
                       inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

                // set trigger mode, LT is by default
                ev.events = EPOLLIN | EPOLLET;    // edge-triggered
                ev.data.fd = cliSocket;
                // add a new connect socket fd to event poll list
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cliSocket, &ev) == -1) {
                    printf("epoll_ctl: cliSocket error\n");
                    return -1;
                }
            } else {
                cliSocket = readyEvents[n].data.fd;

                memset(buffer, 0, BUFF_SIZE);
                nbytes = recv(cliSocket, buffer, sizeof(buffer), 0);
                if (nbytes < 0) {
                    printf("recv error\n");
                    continue;
                } else if (nbytes == 0) {
                    printf("\nDisconnect fd[%d]\n", cliSocket);
                    close(cliSocket);
                    // close socket fd, epoll will remove this descriptor automatically
                    // option use EPOLL_CTL_DEL to remove manually
                } else {
                    printf("From fd[%d]\n", cliSocket);
                    printf("Recv: %sLength: %d\n\n", buffer, nbytes);
                }
            }
        }
    }

    //    return 0;
}
