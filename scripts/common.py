#
# common.py
#

import emb
import re

def BaseModule(object):

    def respond(self, msg):
        emb.msg_send(msg)
    

