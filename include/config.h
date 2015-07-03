#ifndef CONFIG_H
#define CONFIG_H

struct config {
    char *nick;
    char *addr;
    char *port;
    char *home;
    char **masters;
};

#define CNICK 0x1
#define CADDR 0x2
#define CPORT 0x4
#define CHOME 0x8
#define CMSTR 0x10
#define CFULL 0x1f

struct config *parse_args(int argc, char *argv[]);

#endif /* CONFIG_H */

