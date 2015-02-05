#!/bin/bash
HOST='10.35.12.2'
USER='anonymous'
PASSWD=''

# Add file names here to send them
FILESRC=('RobotSettings.txt' 'GUISettings.txt')
PATHDEST=('/home/lvuser' '/home/lvuser')
FILEDEST=('RobotSettings.txt' 'GUISettings.txt')

# script: login
echo "quote USER $USER
quote PASS $PASSWD" > /tmp/ftp.$$

# script: send files in list
for (( i=0 ; i < ${#FILESRC[@]} ; i++ ));
do
    echo "cd ${PATHDEST[$i]}" >> /tmp/ftp.$$
    echo "put ${FILESRC[$i]} ${FILEDEST[$i]}" >> /tmp/ftp.$$
done

# script: logout of FTP server
echo "quit" >> /tmp/ftp.$$

# execute script
ftp -ivn $HOST < /tmp/ftp.$$

# delete script and exit
rm /tmp/ftp.$$
exit 0

