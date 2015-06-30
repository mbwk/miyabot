#
# multiplexer.py
# -
# a python script for miyabot that handles python extensions for the bot
#

import emb

def msgcheck(target, usr, msg):
    message = str(msg)
    if "greetings" in message:
        response = "hello, {}".format(usr)
        emb.msg_send(target, response)

def helloworld():
    print("hello world!")

