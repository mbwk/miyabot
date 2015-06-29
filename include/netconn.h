#ifndef NETCONN_H
#define NETCONN_H

#include <inttypes.h>

int NET_establish(char *addr, char *port);
int NET_terminate(int sockdes);
void NET_sendmesg(int sockdes, char *msgbuff);
int NET_recvmesg(int sockdes, char **ibuff);

#endif /* NETCONN_H */

