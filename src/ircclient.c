#include "Python.h"

#include "ircclient.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h> /* for tolower() */

#include "netconn.h"

#define PY_MPLX "multiplexer"
#define PY_FUNC "msgcheck"

char obuff[513];
int sockdes;
int running = 0;

char *chan;
char *mstr;

enum MSG_type {
    T_PING,
    T_PRVMSG
};

void
IRC_write(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(obuff, 512, fmt, ap);
    va_end(ap);
    NET_sendmesg(sockdes, obuff);
    printf("[srv] << [cli]: %s", obuff);
}

void
IRC_write_privmsg(char *target, char *msg)
{
    IRC_write("PRIVMSG %s :%s\r\n", target, msg);
}

void
IRC_write_privmsg_response(char *target, char *msg, char *user)
{
    IRC_write("PRIVMSG %s :%s: %s\r\n", target, user, msg);
}

int
IRC_pass_to_script(char *target, char *user, char *msg)
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    Py_Initialize();
    pName = PyUnicode_FromString(PY_MPLX);
    pModule = PyImport_Import(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, PY_FUNC);

        pArgs = PyTuple_New(3);

        if (pFunc && PyCallable_Check(pFunc)) {
            PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(target));
            PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(user));
            PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(msg));
        }

        pValue = PyObject_CallObject(pFunc, pArgs);
        Py_DECREF(pArgs);

        if (pValue != NULL) {
            printf("Result of call: %ld\n", PyLong_AsLong(pValue));
            Py_DECREF(pValue);
        } else {
            Py_DECREF(pFunc);
            Py_DECREF(pModule);
            PyErr_Print();
            fprintf(stderr, "Call failed\n");
            return 1;
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", PY_MPLX);
        return 1;
    }

    Py_Finalize();
    return 0;
}

void
IRC_privmsg_interpret(char *user, char *command, char *target, char *message)
{
    int obey = 0;
    int msglen = strlen(message);

    if (command[0] != 'P') {
        return;
    }

    if (!strcmp(user, mstr)) {
        obey = 1;
    }

    /* prompt */
    if (message[0] == '>' && message[1] == '>') {
        int i, inword = 0, haveword = 0;
        char *word;

        for (i = 2; i < msglen; ++i) {
            if (message[i] == ' ') {
                if (inword) {
                    message[i] = '\0';
                    inword = 0;
                }
                continue;
            } else if (!inword) {
                word = message + i;
                inword = 1;
                haveword = 1;
                break;
            } 
        }

        if (haveword) {
            int wordlen = strlen(word);
            for (i = 0; i < wordlen; ++i) {
                tolower(word[i]);
            }

            if (!strncmp(word, "ping", 4)) {
                IRC_write_privmsg(target, "PONG");
                return;
            } else if (!strncmp(word, "die", 3) && obey == 1) {
                running = 0;
                return;
            } else if (!strncmp(word, "privcheck", 9)) {
                if (obey) {
                    IRC_write_privmsg_response(target, "you're privileged", user);
                    return;
                } else {
                    IRC_write_privmsg_response(target, "you have no privilege", user);
                    return;
                }
            }
        }
    }

    IRC_pass_to_script(target, user, message);
}

void
IRC_interpret(char *user, char *command, char *where, char *message)
{
    char *target;
    char *sep;
    if (where == NULL || message == NULL) return;

    if ((sep = strchr(user, '!')) != NULL) {
        user[sep - user] = '\0';
    }

    if (where[0] == '#' || where[0] == '&' || where[0] == '+' || where[0] == '!') {
        target = where;
    } else {
        target = user;
    }

    printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);

    if (target[0] == '#') {
        IRC_privmsg_interpret(user, command, target, message);
    }
}

void
IRC_parse(char *msg)
{
    int i, l = strlen(msg), start;
    int wordcount = 0;
    char *user, *command, *where, *message;
    user = command = where = message = NULL;
    for (i = 1; i < l; ++i) {
        if (msg[i] == ' ') {
            msg[i] = '\0';
            ++wordcount;

            switch (wordcount) {
                case 1:
                    user = msg + 1;
                    break;
                case 2:
                    command = msg + start;
                    break;
                case 3:
                    where = msg + start;
                    break;
            }
            
            if (i == l - 1) continue;

            start = i + 1;
        } else if (msg[i] == ':' && wordcount == 3) {
            if (i < l - 1) message = msg + i + 1;
            break;
        }
    }

    if (wordcount < 2) return;

    if (!strncmp(command, "001", 3) && chan != NULL) {
        IRC_write("JOIN %s\r\n", chan);
    } else if (!strncmp(command, "PRIVMSG", 7) || !strncmp(command, "NOTICE", 6)) {
        IRC_interpret(user, command, where, message);
    }
}

void
IRC_read(char *msg)
{
    char sbuff[513];
    strcpy(sbuff, msg);

    printf("[srv] >> [cli]: %s", sbuff);

    if (!strncmp(sbuff, "PING", 4)) {
        sbuff[1] = 'O';
        IRC_write(sbuff);
    } else if (msg[0] == ':') {
        IRC_parse(sbuff);
    }
}

void
IRC_separate(char *ibuff, int len)
{
    int i, o = -1;
    for (i = 0; i < len; ++i) {
        char msgbuff[512];
        ++o;
        msgbuff[o] = ibuff[i];

        if ((i > 0 && ibuff[i] == '\n' && ibuff[i - 1] == '\r') || o == 512) {
            msgbuff[o + 1] = '\0';
            IRC_read(msgbuff);
            o = -1;
        }
    }

}

void
IRC_run(char *nick, char *addr, char *port, char *channel, char *master)
{
    int msglen;
    char *ibuff;

    chan = channel;
    mstr = master;

    sockdes = NET_establish(addr, port);
    IRC_write("USER %s 0 0 :%s\r\n", nick, nick);
    IRC_write("NICK %s\r\n", nick);

    running = 1;
    while (running) {
        msglen = NET_recvmesg(sockdes, &ibuff);
        if (msglen < 1) {
            running = 0;
        } else {
            IRC_separate(ibuff, msglen);
        }
    }
}

