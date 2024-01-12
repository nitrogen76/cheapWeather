Sensors I use or have used:

[Acurite 5n1 weather station](https://www.acurite.com/learn/installation/5in1). The temp sensor on this weather station is garbage.  Use one of the other sensors in a solar-shielded housing.  There's an [PRO+ sensor upgrade](https://www.amazon.com/gp/product/B00T0K8MJW/) that makes the temperature swings a bit less pronounced, but I still recommend one of the other Accurite or LaCrosse sensors as an addition to temperature monitoring.

[Ecowitt WS68](https://shop.ecowitt.com/products/ws68). This is a MUCH BETTER wind sensor than the Acurite 5n1.  IT lacks temperature, humidity, and rain, but the wind direction sensor works better.  It also has a Lux/UV meter but rtl_433's support for it is lacking at the moment.  (We're working on that, though.)  Even though the sensor says it does not work alone, you can use it via rtl_433 without any other equipment.

[Acurite 6045M Lightning Sensor](https://www.acurite.com/lightning-detector-with-temperature-and-humidity.html) Get this [Solar Shield](https://www.amazon.com/gp/product/B01M64ISDE/) for it, it's a better temp sensor than the 5n1

[LaCrosse TX141THBv2](https://www.lacrossetechnology.com/products/tx141th-bv2) This is an old sensor that works VERY WELL with [this solar shield](https://www.amazon.com/gp/product/B00VSXENM4/) and is, strangely enough, the sensor I use for temperature.

[ECOWITT WH5360B Rain Gauge](http://www.ecowitt.com/Rain_Gauge/) Not that impressed with it's accuracy.  The [Acurite 5n1](https://www.acurite.com/learn/installation/5in1) rain gauge is better, and tracks with a neighbor's rain gauge.

[Raspberry Pi SenseHat](https://www.raspberrypi.org/products/sense-hat/) (Don't use these, they are garbage) (Not via rtl433)

[Dracal USB Precision Barometers](https://www.dracal.com/store/products/usb_bar20/index.php)  (You should be able to use *ANY* of their sensors that use USBTENKIGET.  Use these instead of the [Raspberry Pi SenseHat](https://www.raspberrypi.org/products/sense-hat/).) (Not via rtl433)

GMC 320 geiger counter from GQ  (Not via rtl433)

[AirGradient air quality Sensors](https://www.airgradient.com/) They currently have an API that is scrapable via telegraf http input plugin durectly into influxdb. (not via rtl433)
There are custom firmwares that will allow you to scrape directly.  Also, MQTT is on the roadmap for newer sensors.


Any sensor that is powered by rtl_433 should work just fine.  Also, any sensor you can get data from and into influxDB is easy.  It took me 20 minutes to write code to get the Dracal barometer to work.
