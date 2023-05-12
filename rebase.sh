#!/bin/bash

# fetch from https://crrev.com/c/4528303, modify the patchset number below
# git fetch https://chromium.googlesource.com/angle/angle refs/changes/03/4528303/24 && git cherry-pick FETCH_HEAD

# rebase the current commit onto the base in android tree, modify the android
# tree top hash below.
git rebase --onto 6a09e41ce6ea HEAD^
