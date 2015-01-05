#!/bin/bash
NAME=${0##*/}

args() {
  echo >&2 "$@"
  echo usage: $NAME COMMAND
  echo -e "COMMAND can be either 'format' or 'clean'"

  echo $NAME format
  echo -e "\tformat C++ code\n"
}

format() {
  find src -type f -name \*.cpp -o -name \*.hpp -o -name \*.inl | uncrustify -c uncrustify.cfg -F - -l CPP --replace --no-backup
}

if [ $# = 0 ] ; then
  format
elif [ $# = 1 ] ; then
  if [ "$1" = "format" ] ; then
    format
  else
    args $NAME: invalid command \'$1\'
  fi
else
  args $NAME: wrong number of args \(expected 1\; got $#\)
fi

