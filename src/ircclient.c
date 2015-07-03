#include "Python.h"

#include "ircclient.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h> /* for tolower() */

#include "netconn.h"

#define PY_NAME "scripts"
#define PY_FUNC "msgcheck"

char obuff[513];
int sockdes;
int running = 0;

char *chan;
char **masters;

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

static int numargs = 0;

static PyObject*
emb_msg_send(PyObject *self, PyObject *args)
{
    char *target, *msg;
    if (!PyArg_ParseTuple(args, "ss", &target, &msg)) {
        return NULL;
    }
    IRC_write_privmsg(target, msg);
    return PyLong_FromLong(numargs);
}

static PyMethodDef EmbMethods[] = {
    {"msg_send", emb_msg_send, METH_VARARGS, "Sends a message"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef EmbModule = {
    PyModuleDef_HEAD_INIT, "emb", NULL, -1, EmbMethods,
    NULL, NULL, NULL, NULL
};

static PyObject*
PyInit_emb(void)
{
    return PyModule_Create(&EmbModule);
}

int
IRC_pass_to_script(char *target, char *user, char *msg)
{
    setenv("PYTHONPATH", ".", 1);

    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    PyImport_AppendInittab("emb", &PyInit_emb);
    Py_Initialize();
    pName = PyUnicode_FromString(PY_NAME);
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
        fprintf(stderr, "Failed to load \"%s\"\n", PY_NAME);
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

    for (int i = 0; masters[i] != NULL; ++i) {
        char *mstr = masters[i];
        if (!strcmp(user, mstr)) {
            obey = 1;
            break;
        }
    }

    /* prompt */
    if (message[0] == '>' && message[1] == '>') {
        int i, inword = 0, haveword = 0;
        int lettercount = 0, wordstart = 0;

        for (i = 2; i < msglen; ++i) {
            if (message[i] == ' ') {
                if (inword) {
                    inword = 0;
                    break;
                }
                continue;
            } else if (!inword) {
                wordstart = i;
                inword = 1;
                haveword = 1;
            } 

            if (inword) {
                ++lettercount;
            }
        }

        char word[lettercount + 1];
        strncpy(word, message + wordstart, lettercount);
        word[lettercount] = '\0';


        if (haveword) {
            int wordlen = strlen(word);
            for (i = 0; i < wordlen; ++i) {
                word[i] = tolower(word[i]);
            }

            if (!strncmp(word, "ping", 4)) {
                IRC_write_privmsg(target, "PONG");
                return;
            } else if (!strncmp(word, "die", 3)) {
                if (obey) {
                    running = 0;
                } else {
                    IRC_write_privmsg_response(target, "nice try, meatbag", user);
                }
                return;
            } else if (!strncmp(word, "privcheck", 9)) {
                if (obey) {
                    IRC_write_privmsg_response(target, "you're privileged", user);
                } else {
                    IRC_write_privmsg_response(target, "you have no privilege", user);
                }
                return;
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
    int i, l = strlen(msg), start = 0;
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
    } else if (!strncmp(command, "INVITE", 6)) {
        IRC_write("JOIN %s\r\n", message);
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
IRC_connect(char *addr, char *port, char *nick)
{
    sockdes = NET_establish(addr, port);
    IRC_write("USER %s 0 0 :%s\r\n", nick, nick);
    IRC_write("NICK %s\r\n", nick);
}

void
IRC_run(char *nick, char *addr, char *port, char *channel, char **masters_arr)
{
    int msglen;
    char *ibuff;

    chan = channel;
    masters = masters_arr;

    IRC_connect(addr, port, nick);
    /*
    sockdes = NET_establish(addr, port);
    IRC_write("USER %s 0 0 :%s\r\n", nick, nick);
    IRC_write("NICK %s\r\n", nick);
    */

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

