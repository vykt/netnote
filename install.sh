#!/bin/sh

INSTALL_DIR=/usr/local/bin
EXAMPLE=/home/vykt/programming


#Check installation
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

#create netnote group
groupadd netnote

#copy netnote.conf
cp netnote.conf /etc

#copy netnote (to /usr/local/bin by default)
cp ./netnote ${EXAMPLE}
