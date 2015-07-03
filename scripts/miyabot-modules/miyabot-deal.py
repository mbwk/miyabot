def hear(bot, msg):
    if "the special deal" in msg:
        bot.msg("only 3 pounds 99 a month")
    elif "respond to me" in msg:
        bot.res("okay")
    elif "message me" in msg:
        bot.msg("okay")

def prompt(bot, msg):
    bot.res("了解さ")

