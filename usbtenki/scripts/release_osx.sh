#!/bin/bash

QTDIR=$HOME/Qt/5.11.2/clang_64/bin

VERSION_H=common/usbtenki_version.h
LIBUSB_DYLIB=../../libusb/Xcode/build/Release/libusb-1.0.0.dylib

if [ ! -f $VERSION_H ]; then
	echo "Please run this script from its parent directory."
	exit 1
fi


VERSION=`grep USBTENKI_VERSION $VERSION_H | head -1 | cut -d \" -f 2`
APP=qtenki.app
PATH=$PATH:$QTDIR
#otool -L $(APP)
echo "Version: $VERSION"

cd client || exit 1

if [ ! -e $LIBUSB_DYLIB ]; then
	echo "Could not find $LIBUSB_DYLIB"
	exit 1
fi

Make -f Makefile.osx clean all || exit 1

cd ..

cd qtenki || exit 1

iconutil --convert icns qtenki.iconset --output qtenki.icns || exit 1
qmake || exit 1
make || exit 1
macdeployqt $APP || exit 1
cp $LIBUSB_DYLIB $APP/Contents/Frameworks || exit 1

rm -rf osx_staging
rm tmp.dmg

mkdir osx_staging
cp -R qtenki.app osx_staging
ls osx_staging
ln -s /Applications osx_staging/Applications
mkdir osx_staging/.background
cp osx_install_bg.png osx_staging/.background/bg.png

cp ../client/usbtenkiget ./osx_staging/qtenki.app/Contents/MacOS
cp ../client/usbtenkiset ./osx_staging/qtenki.app/Contents/MacOS
install_name_tool -add_rpath "@executable_path/../Frameworks" ./osx_staging/qtenki.app/Contents/MacOS/usbtenkiget
install_name_tool -add_rpath "@executable_path/../Frameworks" ./osx_staging/qtenki.app/Contents/MacOS/usbtenkiset

hdiutil create -srcfolder osx_staging -size 60m -fs HFS+ -volname "QTenki" -fsargs "-c c=64,a=16,e=16" -format UDRW tmp.dmg
hdiutil attach tmp.dmg

sleep 2

echo '
	tell application "Finder"
		tell disk "QTenki"
			open
			set current view of container window to icon view
			set toolbar visible of container window to false
			set statusbar visible of container window to false
			set bounds of container window to { 400, 100, 920, 440 }
			delay 1
			set theViewOptions to the icon view options of container window
			set arrangement of theViewOptions to not arranged
			set icon size of theViewOptions to 72
			set background picture of theViewOptions to file ".background:bg.png"
			delay 1
			set file_list to every file
			repeat with i in file_list
				delay 1
				if the name of i is "Applications" then
					set position of i to { 360, 205 }
				else if the name of i ends with ".app" then
					set position of i to { 160, 205 }
				end if
			end repeat
			close
			open
			update without registering applications
			delay 4
		end tell
	end tell
	' | osascript

sync

DEVS=$(hdiutil attach tmp.dmg | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')

hdiutil detach $DEV
rm ../qtenki-$VERSION.dmg
hdiutil convert tmp.dmg -format UDZO -o ../qtenki-${VERSION}.dmg

cd ..

echo "OK"

