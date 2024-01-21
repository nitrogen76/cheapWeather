#ifndef _tenki_math_device_h__
#define _tenki_math_device_h__

#include "TenkiDevice.h"
#include <QString>
#include <QScriptEngine>

class TenkiMathDevice : public TenkiDevice
{
	public:
		TenkiMathDevice();
		~TenkiMathDevice();

		int updateChannelData();

		void setChannelEquation(int id, QString &str);
		QString getChannelEquation(int chn);

		void setChannelUnits(int id, int units);
		int getChannelUnits(int id);

	private:
		QString substvars(QString in);

		QString getChannelSettingsKey(int chn, bool units);
		QString equationsStrings[MAX_CHANNELS];
		QScriptEngine engine;
};

#endif // _tenki_math_device_h__

