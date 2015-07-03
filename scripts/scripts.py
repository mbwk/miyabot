#
# scripts.py
# -
# a python script for miyabot that handles python extensions for the bot
#

import os
import importlib.machinery
import emb

class Robot:

    def __init__(self, target):
        self.target = target

    def send_response(self, msg):
        emb.msg_send(self.target, msg)

def send_response(target, msg):
    emb.msg_send(target, msg)

def msgcheck(target, usr, msg):

    bot = Robot(target)

    # put dir of scripts
    dir = "/home/miyabe/Programming/CeeSepples/miyabot/scripts/miyabot-modules" #os.path.dirname(os.path.abspath(__file__))

    # files
    files = os.listdir(dir)
    print(files)

    modules = []

    # importing
    for file in files:
        name = os.path.splitext(file)[0]
        ext = os.path.splitext(file)[1]
        if ext == ".py":
            print(file)
            mod_path = os.path.join(dir, file)
            print(mod_path)
            try:
                loader = importlib.machinery.SourceFileLoader(name, mod_path)
            except:
                raise
            module = loader.load_module()
            module.execute(bot.send_response, msg)


