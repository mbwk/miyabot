#
# scripts.py
# -
# a python script for miyabot that handles python extensions for the bot
#

import emb

def gen_modules():
    return 4

def bot_prompt():
    return 1

def bot_hear():
    return 2

def send_response(msg):
    emb.msg_send(target, response)

def msgcheck(target, usr, msg):
    message = str(msg)

    bot_modules = gen_modules()
    for mod in bot_modules:
        if mod.has_response(message):
            send_response(mod.get_response(message))

