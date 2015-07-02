#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define MSGBUFF_MAX 512

char sbuff[MSGBUFF_MAX + 1];

int
NET_establish(char *addr, char *port)
{
    int sockdes = 0, errno = 0;
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(addr, port, &hints, &res);

    sockdes = socket(res->ai_family, res->ai_socktype, 0);
    errno = connect(sockdes, res->ai_addr, res->ai_addrlen);

    if (errno != 0) {
        printf("%d\n", errno);
    }

    return sockdes;
}

int
NET_terminate(int sockdes)
{
    return sockdes - sockdes;
}

void
NET_sendmesg(int sockdes, char *msgbuf)
{
    write(sockdes, msgbuf, strlen(msgbuf));
}

int
NET_recvmesg(int sockdes, char **ibuff)
{
    int msglen;
    msglen = read(sockdes, sbuff, MSGBUFF_MAX);
    sbuff[msglen] = '\0';
    sbuff[MSGBUFF_MAX] = '\0';
    *ibuff = sbuff;
    return msglen;
}

