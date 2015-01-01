#!/bin/bash

# tabs to 4 spaces
find $PWD -not \( -name .svn -prune -o -name .git -prune -o -name ".*" -prune -o -name Makefile -prune \) ! -type d ! -name _tmp_ -exec sh -c 'permission=`stat -c %a {}` && expand -t 4 {} > _tmp_ && mv _tmp_ {} && chmod $permission {}' \;

# remove trailing whitespace
find $PWD -not \( -name .svn -prune -o -name .git -prune \) -type f -print0 | xargs -0 sed -i -E "s/[[:space:]]*$//"

