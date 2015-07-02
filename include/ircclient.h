#ifndef IRCCLIENT_H
#define IRCCLIENT_H

void IRC_connect(char *addr, char *port, char *nick);
int IRC_listen(char **ibuff);
void IRC_unpack(char *ibuff, int msglen);
void IRC_write_privmsg(char *target, char *msg);
void IRC_write_privmsg_response(char *target, char *msg, char *user);
void IRC_run(char *nick, char *addr, char *port, char *channel, char **masters_arr);

#endif /* IRCCLIENT_H */

