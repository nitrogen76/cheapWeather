#!/bin/bash

MXE_ROOT=/home/mxe/mxe

QMAKE="$MXE_ROOT/usr/i686-w64-mingw32.static/qt/bin/qmake"
VERSION_H=../common/usbtenki_version.h
VERSION=`grep USBTENKI_VERSION $VERSION_H | head -1 | cut -d \" -f 2`
TMPDIR="./tmp"

# Provide a way to manually force a version (for test builds, etc)
if [[ $# > 0 ]]; then
	VERSION=$1
fi

function errorexit
{
	echo -e "\033[31;1m*** ERROR ***: \033[0m" $2
	exit $1
}

function build_commandline_tools
{
	make -C ../client -f ../client/Makefile.mxe clean || errorexit 1 "Command-line tool pre-build cleanup failed"
	make -C ../client -f ../client/Makefile.mxe all -j2 || errorexit 1 "Command-line tool build error"
}

function build_qtenki
{
	$QMAKE -makefile -o ../qtenki/Makefile ../qtenki/qtenki.pro || errorexit 1 "Qmake error"
	make -C ../qtenki clean || errorexit 1 "Command-line tool pre-build cleanup failed"
	make -C ../qtenki -f ../qtenki/Makefile release -j2 || errorexit 1 "QTenki build error"
}

echo "Checking for dependencies..."

command -v makensis || errorexit 1 "makensis not found"
command -v $QMAKE || errorexit 1 "qmake not found : $QMAKE"
[ -d ../../libusb ] || errorexit 1 "Expected libusb-win32-device-bin-0.1.12.2 at ../../libusb (renamed or symlink)"
[ -d ../../libusb/lib ] || errorexit 1 "Expected libusb-win32-device-bin-0.1.12.2 at ../../libusb (renamed or symlink)"
[ -d ../../libusb/bin ] || errorexit 1 "Expected libusb-win32-device-bin-0.1.12.2 at ../../libusb (renamed or symlink)"

echo $MXE_ROOT
echo $VERSION

build_commandline_tools
build_qtenki

rm -rf $TMPDIR
mkdir $TMPDIR
cp ../client/usbtenkiget.exe ../client/usbtenkiset.exe $TMPDIR
cp ../qtenki/release/qtenki.exe $TMPDIR
cp ../qtenki/gpl.txt $TMPDIR/license.txt
cp ../../libusb/bin/libusb0.dll $TMPDIR
cp -r ../windows_driver $TMPDIR/usb_driver
cp qtenki_mxe.nsi $TMPDIR
mkdir $TMPDIR/sources
./release.sh $TMPDIR/sources $VERSION

makensis -DVERSION=$VERSION $TMPDIR/qtenki_mxe.nsi

pwd
