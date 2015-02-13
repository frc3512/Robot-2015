#!/bin/bash
HOST='10.35.12.2'
USER='anonymous'
PASSWD=''

if [ $# -ne 1 ] ; then
    echo usage: ./deploy.sh DIRECTORY
    echo -e "DIRECTORY is the directory in which the robot code binary is located"
fi

# Add file names here to send them
FILESRC=($1/'FRCUserProgram')
PATHDEST=('/home/lvuser')
FILEDEST=('FRCUserProgram')

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

