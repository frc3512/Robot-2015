#!/bin/bash
HOST='10.35.12.2'
USER='lvuser'
PASSWD=''

# Add file names here to send them
FILESRC=('RobotSettings.txt' 'GUISettings.txt')
PATHDEST=('/home/lvuser' '/home/lvuser')
FILEDEST=('RobotSettings.txt' 'GUISettings.txt')

# script: send files in list
for (( i=0 ; i < ${#FILESRC[@]} ; i++ ));
do
    scp ${FILEDEST[$i]} $USER@$HOST:${PATHDEST[$i]}
done

