Sensors I use or have used:

[Acurite 5n1 weather station](https://www.acurite.com/learn/installation/5in1)

[Acurite 6045M Lightning Sensor](https://www.acurite.com/lightning-detector-with-temperature-and-humidity.html)

[ECOWITT WH5360B Rain Gauge](http://www.ecowitt.com/Rain_Gauge/)
[Raspberry Pi SenseHat](https://www.raspberrypi.org/products/sense-hat/) (Don't use these, they are garbage) (Not via rtl433)

[Dracal USB Precision Barometers](https://www.dracal.com/store/products/precision_barometer/index.php)  (You should be able to use *ANY* of their sensors that use USBTENKIGET.  Use these instead of the SenseHat.) (Not via rtl433)

Any sensor that is powered by rtl_433 should work just fine.  Also, any sensor you can get data from and into influxDB is easy.  It took me 20 minutes to write code to get the Dracal barometer to work.
