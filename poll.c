#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <poll.h>

#define BACKLOG      5
#define BUFF_SIZE    200
#define DEFAULT_PORT 6666
#define OPEN_MAX     1024

int main(int argc, char** argv)
{
    int SERV_PORT = DEFAULT_PORT;

    if (argc > 2) {
        printf("param err:\nUsage:\n\t%s port | %s\n\n", argv[0], argv[0]);
        return -1;
    }

    if (argc == 2) {
        SERV_PORT = atoi(argv[1]);
    }

    printf("Listen Port: %d\nListening ...\n", SERV_PORT);

    int i, maxi, nready;
    int servSocket, cliSocket;
    ssize_t nbytes;
    char buf[BUFF_SIZE];
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cliAddr, servAddr;

    if ((servSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket\n");
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
    servAddr.sin_port        = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(servSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("bind\n");
        return -1;
    }

    if (listen(servSocket, BACKLOG) < 0) {
        printf("listen err\n");
        return -1;
    }

    // add listen socket fd to client buffer[0]
    client[0].fd = servSocket;
    client[0].events = POLLRDNORM;

    for (i = 1; i < OPEN_MAX; i++) {
        client[i].fd = -1; /* -1 indicates available entry */
    }
    maxi = 0;              /* max index into client[] array */

    for (;;) {
        int timeout_ms = 2000;
        nready = poll(client, maxi + 1, timeout_ms);
        if (nready < 0) {
            printf("poll err\n");
            return -1;
        } else if (nready == 0) {
            printf("poll timeout\n");
            continue;
        }

        // check a new accept socket fd from client buffer[0]
        if (client[0].revents & POLLRDNORM) {
            socklen_t addrLen = sizeof(cliAddr);
            if ((cliSocket
                 = accept(servSocket, (struct sockaddr*)&cliAddr, &addrLen))
                < 0) {
                printf("accept err\n");
                return -1;
            }

            for (i = 1; i < OPEN_MAX; i++) {
                if (client[i].fd < 0) {
                    // save connect socket fd to client buffer
                    client[i].fd = cliSocket;
                    /* Normal data may be read. */
                    client[i].events = POLLRDNORM;
                    break;
                }
            }

            printf("\nNew client connections client[%d] %s:%d\n", i,
                   inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

            if (i == OPEN_MAX) {
                printf("too many clients\n");
            }

            if (i > maxi) {
                maxi = i; /* max index in client[] array */
            }

            if (--nready <= 0) {
                continue; /* no more readable descriptors */
            }
        }

        // check all connect client socket fd
        for (i = 1; i <= maxi; i++) {
            if ((cliSocket = client[i].fd) < 0) {
                continue;
            }

            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                memset(buf, 0, BUFF_SIZE);
                nbytes = recv(cliSocket, buf, BUFF_SIZE, 0);
                if (nbytes < 0) {
                    printf("recv err\n");
                    continue;
                } else if (nbytes == 0) {
                    printf("client[%d] closed connection\n", i);
                    close(cliSocket);
                    client[i].fd = -1;
                } else {
                    printf("\nFrom client[%d]\n", i);
                    printf("Recv: %sLength: %d\n\n", buf, (int)nbytes);
                }

                if (--nready <= 0) {
                    break; /* no more readable descriptors */
                }
            }
        }
    }
}
