#!/usr/bin/env python

# Copyright 2018 Google Inc. All Rights Reserved.

"""A Script to update git submodules. Should be run from roo"""

import sys
import os
from subprocess import call

def printHelp():
    print ("Usage: python git_submodule_update.py <git_dir>")
    print ("\n In the given <git_dir> execute the command 'git submodule update --init --recursive'")

if __name__ == "__main__":
    if (2 != len(sys.argv)):
        printHelp()
        sys.exit(1)
    elif (not os.path.isdir(sys.argv[1])):
        print ("Specified git path, %s, does not exist!" % (sys.argv[1]))
        sys.exit(1)
    # We have a good dir, execute git submodule update cmd there
    print ("Executing command 'git submodule update --init --recursive' in dir '%s'" % (sys.argv[1]))
    os.chdir(sys.argv[1])
    if (0 != call(["git", "submodule", "update", "--init", "--recursive"])):
        print ("git submodule update command failed. There may not be submodules in the specified dir")
        sys.exit(1)
    print ("git submodule update command succeeded!")
    sys.exit(0)
