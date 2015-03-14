#!/bin/bash
HOST='10.35.12.2'
USER='lvuser'

if [ $# -ne 1 ] ; then
    echo usage: ./deploy.sh DIRECTORY
    echo -e "DIRECTORY is the directory in which the robot code binary is located"
    exit 1
fi

ssh $USER@$HOST "killall FRCUserProgram"
scp $1/FRCUserProgram $USER@$HOST:/home/lvuser
ssh $USER@$HOST "/home/lvuser/FRCUserProgram &"

