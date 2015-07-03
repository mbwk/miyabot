from collections import OrderedDict
import time


def remove_duplicates(lst):
    return list(OrderedDict.fromkeys(lst))

def time_stamp():
    return str(int(time.time()))

def execute(bot, msg):
    bot("hello world")

