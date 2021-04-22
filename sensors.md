Sensors I use or have used:

[Acurite 5n1 weather station](https://www.acurite.com/learn/installation/5in1). The temp sensor on this weather station is garbage.  Use one of the other sensors in a solar-shielded housing.  There's an [PRO+ sensor upgrade](https://www.amazon.com/gp/product/B00T0K8MJW/) that I will test to see if it improves.

[Acurite 6045M Lightning Sensor](https://www.acurite.com/lightning-detector-with-temperature-and-humidity.html) Get this [Solar Shield](https://www.amazon.com/gp/product/B01M64ISDE/) for it, it's a better temp sensor than the 5n1

[LaCrosse TX141THBv2](https://www.lacrossetechnology.com/products/tx141th-bv2) This is an old sensor that works VERY WELL with [this solar shield](https://www.amazon.com/gp/product/B00VSXENM4/) and is, strangely enough, the sensor I use for temperature.

[ECOWITT WH5360B Rain Gauge](http://www.ecowitt.com/Rain_Gauge/) Not that impressed with it's accuracy.  The [Acurite 5n1](https://www.acurite.com/learn/installation/5in1) rain gauge is better, and tracks with a neighbor's rain gauge.

[Raspberry Pi SenseHat](https://www.raspberrypi.org/products/sense-hat/) (Don't use these, they are garbage) (Not via rtl433)

[Dracal USB Precision Barometers](https://www.dracal.com/store/products/usb_bar20/index.php)  (You should be able to use *ANY* of their sensors that use USBTENKIGET.  Use these instead of the [Raspberry Pi SenseHat](https://www.raspberrypi.org/products/sense-hat/).) (Not via rtl433)

Any sensor that is powered by rtl_433 should work just fine.  Also, any sensor you can get data from and into influxDB is easy.  It took me 20 minutes to write code to get the Dracal barometer to work.
