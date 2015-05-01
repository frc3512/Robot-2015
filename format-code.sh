#!/bin/bash
# skips .svn, .hg, and .git source control metadata folders, hidden files (.*),
# and Makefiles since tabs would be replaced

# format C++ code
find $PWD ! -type d -not \( -name .svn -prune -o -name .hg -prune -o -name .git -prune -o -name ".*" -prune -o -name Makefile -prune \) -a \( -name \*.cpp -o -name \*.hpp -o -name \*.inl \) | uncrustify -c uncrustify.cfg -F - -l CPP --replace --no-backup

# format C code
find $PWD ! -type d -not \( -name .svn -prune -o -name .hg -prune -o -name .git -prune -o -name ".*" -prune -o -name Makefile -prune \) -a \( -name \*.c -o -name \*.h \) | uncrustify -c uncrustify.cfg -F - -l C --replace --no-backup

