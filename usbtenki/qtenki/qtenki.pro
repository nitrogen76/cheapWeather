CONFIG += release
TEMPLATE = app
TARGET = qtenki
DEPENDPATH += .
INCLUDEPATH += . ../common ../library
RESOURCES	= qtenki.qrc

win32 {
	INCLUDEPATH += ../../libusb/include
	DEFINES += WINDOWS_VERSION USE_OLD_LIBUSB
	LIBS += -L../../libusb/lib/gcc -lusb
	RC_FILE = qtenki.rc
}

macx {
	LIBS += ../../libusb/Xcode/build/Release/libusb-1.0.0.dylib
	INCLUDEPATH += ../../libusb/libusb
	ICON = qtenki.icns
}

unix:!macx {
	CONFIG+=link_pkgconfig
	PKGCONFIG+=libusb-1.0
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport 

QT += script

# Input

HEADERS += ../library/usbtenki.h ../library/convert_type_k.h ../common/usbtenki_cmds.h ../common/usbtenki_version.h About.h \
			 TextViewer.h TenkiSources.h DashSensor.h TenkiDashboard.h TenkiDevice.h Logger.h \
			DataSourceCheckBox.h SimpleLogger.h MainWindow.h BigView.h GraphView.h BigLabel.h SourceAliasEdit.h \
			PowerPreference.h CurrentPreference.h TemperaturePreference.h PressurePreference.h FrequencyPreference.h ConfigCheckbox.h \
			VoltagePreference.h ConfigPanel.h SelectableColor.h globals.h qcustomplot.h GraphLegendPreference.h single_application.h LengthPreference.h \
			MinMaxResettable.h ThermocoupleColorPreference.h ConcentrationPreference.h DashSensorDevice.h TenkiMathDevice.h DashSensorMath.h EditButton.h \
			MathEditDialog.h UnitsDropDown.h GraphLineStyleDialog.h LineStyleDropDown.h HTMLViewer.h


SOURCES += ../library/usbtenki.c ../library/convertRaw.c ../library/convert_type_k.c main.cpp \
			TextViewer.cpp TenkiSources.cpp SimpleLogger.cpp DashSensor.cpp TenkiDashboard.cpp \
			TenkiDevice.cpp Logger.cpp DataSourceCheckBox.cpp About.cpp MainWindow.cpp BigView.cpp GraphView.cpp \
			PowerPreference.cpp CurrentPreference.cpp VoltagePreference.cpp TemperaturePreference.cpp PressurePreference.cpp FrequencyPreference.cpp ConfigCheckbox.cpp \
			SourceAliasEdit.cpp BigLabel.cpp ConfigPanel.cpp SelectableColor.cpp globals.cpp qcustomplot.cpp GraphLegendPreference.cpp single_application.cpp \
			LengthPreference.cpp MinMaxResettable.cpp ThermocoupleColorPreference.cpp ConcentrationPreference.cpp DashSensorDevice.cpp TenkiMathDevice.cpp DashSensorMath.cpp \
			EditButton.cpp MathEditDialog.cpp UnitsDropDown.cpp GraphLineStyleDialog.cpp LineStyleDropDown.cpp HTMLViewer.cpp

