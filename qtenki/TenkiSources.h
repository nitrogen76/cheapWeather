#ifndef _tenki_sources_h__
#define _tenki_sources_h__


#include <QThread>
#include <QTimer>
#include <QList>
#include "TenkiDevice.h"
#include "usbtenki.h"

struct sourceDescription {
	char name[32]; // XXXXXX:XX form.
	QString q_name;
	QString q_alias;
	int chn_id;
	
	int chip_id;
	QString chipShortString;
	QString chipString;

	TenkiDevice *td;
	USBTenki_channel *chn_data;
};

class TenkiSourceAddRemove
{
	public:
		virtual void addTenkiSource(struct sourceDescription *sd) = 0;
		virtual void removeTenkiSource(struct sourceDescription *sd) = 0;
};


class TenkiSources : public QObject
{
	Q_OBJECT

	public:
		TenkiSources();
		~TenkiSources();
		int init();
		void start();
		int getNumDevices();

		// Add all known TenkiDevices to target
		void syncDevicesTo(TenkiDeviceAddRemove *tdr);
		void addSourcesTo(TenkiSourceAddRemove *tsar);

		void formatValue(QString *str, float value);

		struct sourceDescription *getSourceByName(QString src);
		QString getSourceAliasByName(QString src);

		void setTemperatureUnit(int temperature_unit);
		void setPressureUnit(int pressure_unit);
		void setFrequencyUnit(int frequency_unit);
		void setVoltageUnit(int volt_unit);
		void setCurrentUnit(int current_unit);
		void setPowerUnit(int power_unit);
		void setLengthUnit(int length_unit);
		void setConcentrationUnit(int concentration_unit);
		void setDisplayDigits(int digits);
		void setReferenceSeaLevelPressure(double value); // In Pascals
		void setUseIECthermocoupleColors(int use_iec); // otherwise, use ANSI
		int getUseIECthermocoupleColors(void);

		// Return a recent atmostpheric pressure value (used to set altitude reference)
		double getRecentPressure(void); // in Pascals

		int displayDigits();

		void convertToUnits(const struct USBTenki_channel *chn, struct USBTenki_channel *dst);
	protected:

	public slots:
		void doCaptures();
		void updateAlias(QString source_name, QString alias);
		void run();
		void setInterval_ms(int interval);

	signals:
		void newDeviceFound(TenkiDevice *td);
		void deviceGone(TenkiDevice *td);

		void captureCycleCompleted();
		void changed();

	private:
		QThread *thread;
		QTimer *timer;
		QList<TenkiDevice*> device_list;
		QList<struct sourceDescription*> sourceList;
		int scanForDevices();
		int addDeviceSources(TenkiDevice *td);
		int addDeviceSource(TenkiDevice *td, int chn_id, struct USBTenki_channel *chndat);

		struct USBTenki_unitPreferences units;
/*		int pressure_unit;
		int temperature_unit;
		int frequency_unit;
		int volt_unit;
		int current_unit;
		int power_unit;
		int length_unit;*/

		int timer_interval;
		int display_digits;
		int use_iec_thermocouple_colors;
		double recent_pressure_P;
};

#endif // _tenki_sources_h__

