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

    printf("Connect to host %s:%d ...\n", SERVER_IP, SERVER_PORT);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("socket err\n");
        return -1;
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(SERVER_PORT);
//    servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);    //Outdated method
    inet_aton(SERVER_IP, &servAddr.sin_addr);

    if (connect(fd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("connect err\n");

        return -1;
    }

    while (1) {
        static uint32_t count = 1;
        char buf[128] = "Hello Server, I am Client.";
        sprintf(buf + strlen(buf), "%d\n", count++);

        int len = (int)strlen(buf);
        if (send(fd, buf, len, 0) == len) {
            printf("send len %d ok\n", len);
        } else {
            printf("send err\n");
            break;
        }

        sleep(3);
    }

    close(fd);
    return 0;
}
