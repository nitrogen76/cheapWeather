#!/bin/bash

PREFIX=usbtenki
HEXFILE=$PREFIX.hex

if [[ $# > 1 ]]; then
	VERSION=$2
else
	VERSION_H=../common/usbtenki_version.h
	VERSION=`grep USBTENKI_VERSION $VERSION_H | head -1 | cut -d \" -f 2`
fi

echo "Release script for $PREFIX"

if [[ $# < 1 ]]; then
	echo "Syntax: ./release.sh releasedir [version]"
	echo
	echo "ex: './release releasedir' will produce $PREFIX-$VERSION.tar.gz in releasedir out of git HEAD."
	exit
fi

RELEASEDIR=`readlink -f $1`
DIRNAME=$PREFIX-$VERSION
FILENAME=$PREFIX-$VERSION.tar.gz
TAG=v$VERSION

echo "Version: $VERSION"
echo "Filename: $FILENAME"
echo "Release directory: $RELEASEDIR"
echo "--------"
echo "Ready? Press ENTER to go ahead (or CTRL+C to cancel)"

read

if [ -f $RELEASEDIR/$FILENAME ]; then
	echo "Release file already exists!"
	exit 1
fi

cd ..
git tag $TAG -f -a -m "Release $VERSION"
git archive --format=tar --prefix=$DIRNAME/ HEAD | gzip > $RELEASEDIR/$FILENAME


