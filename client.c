#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_PORT 6666

int main(int argc, char* argv[])
{
    char* SERVER_IP = DEFAULT_IP;
    int SERVER_PORT = DEFAULT_PORT;

    if (argc > 3) {
        printf("param err:\nUsage: %s ip port | %s port | %s\n", argv[0],
               argv[0], argv[0]);
        return -1;
    }

    if (argc == 3) {
        SERVER_IP   = argv[1];
        SERVER_PORT = atoi(argv[2]);
    } else if (argc == 2) {
        SERVER_PORT = atoi(argv[1]);
    } else {
        // Do nothing by default
    }

    int fd = -1;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket err\n");
        exit(1);
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(SERVER_PORT);
    //    servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);    //Outdated method
    inet_aton(SERVER_IP, &servAddr.sin_addr);

    if (connect(fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("connect err\n");
        exit(1);
    }

    printf("Connect to host %s:%d ...\n", SERVER_IP, SERVER_PORT);

    while (1) {
        char* buf = "Hello Server, I am Client";
        int len = strlen(buf);
        if (send(fd, buf, len, 0) == len) {
            printf("Send len %d ok\n", len);
        } else {
            printf("send err\n");
            break;
        }

        sleep(3);
    }

    close(fd);
    return 0;
}
