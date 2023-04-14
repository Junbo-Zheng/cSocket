#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/select.h>

#define BACKLOG      5
#define BUFF_SIZE    200
#define DEFAULT_PORT 6666

typedef struct {
    int fd;                  /* client's connection descriptor */
    struct sockaddr_in addr; /* client's address */
} CLIENT;

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

    int i, maxi, maxfd, nready, nbytes;
    int servSocket, cliSocket;

    fd_set allset, rset;

    char buffer[BUFF_SIZE];
    CLIENT client[FD_SETSIZE]; /* FD_SETSIZE == 1024 */
    struct sockaddr_in servAddr, cliAddr;

    if ((servSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    int optval = 1;
    if (setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval))
        < 0) {
        perror("setsockopt");
        return -1;
    }

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_port        = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(servSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("bind");
        return -1;
    }

    if (listen(servSocket, BACKLOG) < 0) {
        printf("listen err");
        return -1;
    }

    maxi = -1;
    maxfd = servSocket;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i].fd = -1; /* -1 indicates available entry */
    }

    FD_ZERO(&allset);
    FD_SET(servSocket, &allset);

    for (;;) {
        rset = allset; // set allset to rset(readable set)

        struct timeval timeout = {
            .tv_sec  = 5,
            .tv_usec = 0
        };

        nready = select(maxfd + 1, &rset, NULL, NULL, &timeout);
        if (nready < 0) {
            printf("select error\n");
            break;
        } else if (nready == 0) {
            printf("select time out\n");
            continue;
        }

        // check a new accept from listen socket fd
        if (FD_ISSET(servSocket, &rset)) {
            socklen_t addrLen = sizeof(cliAddr);
            if ((cliSocket
                 = accept(servSocket, (struct sockaddr*)&cliAddr, &addrLen))
                < 0) {
                printf("accept error\n");
                return -1;
            }

            printf("\nNew client connections %s:%d\n",
                   inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

            // save connect socket to client buffer
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i].fd < 0) {
                    client[i].fd   = cliSocket;
                    client[i].addr = cliAddr;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                printf("too many clients!!!\n");
            }

            // put accept socket fd to allset
            FD_SET(cliSocket, &allset);
            if (cliSocket > maxfd) {
                maxfd = cliSocket; /* for select */
            }

            if (i > maxi) {
                maxi = i; /* max index in client[] array */
            }

            if (--nready <= 0) {
                continue; /* no more readable descriptors */
            }
        }

        // check all read fd  client fd buffer
        for (i = 0; i <= maxi; i++) {
            if ((cliSocket = client[i].fd) < 0) {
                continue;
            }

            if (FD_ISSET(cliSocket, &rset)) {
                memset(buffer, 0, BUFF_SIZE);

                nbytes = (int)recv(cliSocket, buffer, sizeof(buffer), 0);
                if (nbytes < 0) {
                    printf("recv error\n");
                    continue;
                } else if (nbytes == 0) {
                    printf("\nDisconnect %s:%d\n",
                           inet_ntoa(client[i].addr.sin_addr),
                           ntohs(client[i].addr.sin_port));
                    close(cliSocket);
                    FD_CLR(cliSocket, &allset);
                    client[i].fd = -1;
                } else {
                    printf("\nFrom %s:%d\n", inet_ntoa(client[i].addr.sin_addr),
                           ntohs(client[i].addr.sin_port));
                    printf("Recv: %sLength: %d\n\n", buffer, nbytes);
                }

                if (--nready <= 0) {
                    break; /* no more readable descriptors */
                }
            }
        }
    }
}
