# miyabot

A revival of my old miyabot project.  This time in C11.

A simple, self-contained IRC bot that should run on any POSIX-compliant system.

Now extensible via scripting with Python!  Written for Python3, but can be made to work with
Python2 with a few edits.


## Restrictions

1.  Safety and stability.  The bot should be fine left to its own devices for extended periods of time.
2.  No additional hard dependencies.  The bot to be able to run on multiple POSIX-compliant
    systems without issue.


## Current Features

* Socket connection
* Simple IRC client
* Basic message parsing
* Basic user privileges


