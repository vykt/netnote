#!/bin/sh

INSTALL_DIR=/usr/local/bin
DOWNLOAD_DIR=/var/netnote
MAN_DIR=/usr/share/man

#Check installation
#If not running as root:
if [ "$EUID" -ne 0 ]; then
	echo "Please run the install script as root."
	exit 1
fi

#If program has not been built:
if [ ! -f "netnote" ]; then
	echo "'netnote' executable missing. Did you remember to run 'make' first?"
	exit 1
fi

#If install directory doesn't exist:
if [ ! -d "$INSTALL_DIR" ]; then
	echo "${INSTALL_DIR} doesn't exist! Create it or better, change the install directory."
	exit 1
fi

#If man directory doesn't exist:
if [ ! -d "$MAN_DIR" ]; then
	echo "${MAN_DIR} doesn't exist! Check where your man pages are installed."
	exit 1
fi

#create netnote group
groupadd netnote

#create downloads directory
mkdir ${DOWNLOAD_DIR}
chmod 755 ${DOWNLOAD_DIR}
chown root:netnote ${DOWNLOAD_DIR}

#copy netnote.conf
cp ./netnote.conf /etc

#copy netnote (to /usr/local/bin by default)
cp ./netnote ${INSTALL_DIR}

#copy manpages (to /usr/share/man by default)
cp ./man/man1/netnote.1.bz2 ${MAN_DIR}/man1
