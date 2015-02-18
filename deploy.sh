#!/bin/bash
HOST='10.35.12.2'
USER='admin'
PASSWD=''

if [ $# -ne 1 ] ; then
    echo usage: ./deploy.sh DIRECTORY
    echo -e "DIRECTORY is the directory in which the robot code binary is located"
    exit 1
fi

ssh $USER@$HOST 'echo 'kill `ps x | grep /home/lvuser/FRCUserProgram | grep -v grep | head -n 1 | sed 's/^ *//' | cut -d ' ' -f 1`' | bash -s'

scp $1/FRCUserProgram $USER@$HOST:$PATHDEST

ssh $USER@$HOST 'echo "/home/lvuser/FRCUserProgram &" | bash -s'

exit 0

