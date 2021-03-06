#
# scripts.py
# -
# a python script for miyabot that handles python extensions for the bot
#

import os
import importlib.machinery
import emb

class Robot:

    def __init__(self, target, usr):
        self.target = target
        self.usr = usr

    def msg(self, msg):
        emb.msg_send(self.target, msg)

    def res(self, msg):
        emb.msg_send(self.target, "{}: {}".format(self.usr, msg))

def msgcheck(target, usr, msg):

    bot = Robot(target, usr)

    # put dir of scripts
    dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "miyabot-modules")

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
            if msg.startswith(">>") or msg.startswith("#") or msg.startswith("miyabot"):
                module.prompt(bot, msg)
            else:
                module.hear(bot, msg)


