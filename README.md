# cheapWeather
Weather monitoring using Grafana, influxDB and rtl_433

I'm a cheapskate.  I Did not want to buy a weather station with full data export to Weather Underground.

In fact, I started this project because I didn't want to buy one at ALL.  My neighbor had one, and I just wanted to
read the signals from his.  Unfortunately, I learned his station was placed on his roof, which raised the ambient
temps he recorded by 10º-15º.  So I rummaged around and found an old wireless temperature probe...

To make a long story short, I bought a used Acurite 5in1 from Ebay for $30 because it didn't have the panel for it.

I didn't need it.

The beauty of this is, you can record as little or as much information this way as you want.

Just want to graph temperature and humidity?  Grab a LaCrosse sensor and go to town.

Want an ENTIRE weather station?  Grab an Accurate 5in1, and skip the expensive panel.


So, what do you need?

* A linux distribution.  (I will be using Fedora: https://getfedora.org/ )
* A RTL-SDR dongle. https://www.rtl-sdr.com
* rtl-433. https://github.com/merbanan/rtl_433
* influxdb  (Assuming influx 1.8, but 2.0 is possible with substitutions)
* Grafana (7.1 as of this writing, but any version will work)
* A bit of linux knowledge.


How does all this work, exactly?

To be as simple as possible:

Linux -> rtl_433 -> influxdb -> grafana

First thing you need to do is start up a modern linux distribution.
These instructions will be fedora 31 focused, as thats what I use, but should work on any modern Linux distro.


These instructions assume a passing familiarity with Linix, so after you get your favorite distro running,
You first need to download the rtl_433 software.

Instructions for doing so can be found here: https://github.com/merbanan/rtl_433/blob/master/docs/BUILDING.md

For ease of use, the rtl_433 repo is a submodule of mine.


You can do it the easy way if you do not need bleeding edge support:
```
$ sudo dnf copr enable tvass/rtl_433
$ sudo dnf install rtl_433
```
Also, while we're at it, let's get influxDB installed, as well:

For InfluxDB V1, its pretty simple:
```
$ sudo dnf install influxdb
$ sudo
```

Now lets start influxdb, make sure it starts on boot, and set up some users and basic auth.
```
$ sudo systemctl start influxdb
$ sudo systemctl enable influxdb
```

Let's create some users and a db now.
Start up the influx client:
```
$ influx
```
Run the following queries to create users.  Please change the users and passwords to be meaningful.
```
CREATE USER admin WITH PASSWORD 'goodadminpasswd' WITH ALL PRIVILEGES
CREATE USER weather WITH PASSWORD 'goodweatherpassword'

CREATE DATABASE weather
GRANT ALL on "weather" to "weather"
```

Now, using your favorite text editor, edit the influxdb conf file to enable auth:
```
$ vi /etc/influxdb/influxdb.conf
```
Add this section to the [httpd] section.
```
  auth-enabled = true #
```
Now, restart influxDB:
```
$ sudo systemctl restart influxdb
```
--- 

InfluxDB v2 also works, but youi will have to set up v1 compatibility in order to allow these scripts to work.  The FLUX query language is evil incarnate, and I hate it.

The good thing is, you can use v2 and v1 together in v2.  Setting up V2 is beyond the scope of this document right now, but you can infer what you need with the v1 instructions and the influx documentation.

When v3 comes out for open source, we'll see how that works.

---

Next step: rtl_433 software.

You'll have to add your local linux user to the rtlsdr group to be able to reload its driver.

Plug in your RTL-SDR dongle to a USB port on your computer, and hook up a good antenna.
The antenna you need is dependent on the frequency your equipment operates.  Most operates on 433 MHz, 915MHz, others.

Start the scanning process manually to see if you see your equipment.
```
$ rtl_433
```
If it's running on a frequency other than 433MHz, then:
```
$ rtl_433 -f 915M
```
Or what ever frequency you want to scan.

Wait a while.  Hopefully you'll see output with your weather station, and a decode of it's last transmission.
If you see it, go to the next step.  If not:

* Check to make sure you have the correct frequency. (915M vs 915 for instance)
* check to insure you have a good antenna.


Now, let's manually run the program, connecting to influxDB:
```
rtl_433 -M protocol -M level -C si -F "influx://your.influxDB.host:8086/write?db=weather&p=goodweatherpasswd&u=weather"
```
Wait a while, like 10 minutes.

Now let's check influx to see if that data is in there:
```
$ influx
> auth
username: weather
password: <type your password here>
> use weather
Using database weather

> show measurements
```
You should get SOME output, preferable with the name of the device you want.


Now for the fun part.  Grafana installs should be easy.  You can run an older version directly from your Fedora distros, or go relatively new with these instructions:

https://computingforgeeks.com/how-to-install-grafana-on-fedora/


Important info you'll need to set up your datasource:

URL: (that is basically: influx://your.influxDB.host:8086
Access type: Server

Database, user and password.
HTTP Method: POST

I will post separate info with queries and designs I use for my dashboard.

Some additional notes:
Dewpoints are a hack. They are calculated every minute in the influx-wunderground.py script.
If you aren't sending data to wunderground, you can disable this functionality in the influx-calculate script, or the cheapWeather.ini file

The influx-calculate script is the heart of generating other values from weather station values.  I currently have this configured for MY setup.  Soon, this will be more automatic for differing setups, but for now, you might have to edit some code to customize this.  Hopefully it's easy to figure out.

Got panels?  Got good (better?) queries? I'll take 'em!

If you have multiple weather equipment on multiple frequencies, you can handle that 2 ways:
* You can tell rtl_433 to lock down the specific protocls you care about (`-R ${protocolnum}`) then `-E hop`.  `-f` takes multiple frequencies (`-f 915M -f 433M`)
* You can get a second rtl-sdr dongle and run 2 versions of rtl_433 at the same time.  If you do this, you'll need to use rtl_eerpom to set a serial number, then specify the specific radio/antenna pair with `-d :serialnum` (the colon is important)

* I've included an example of my commandline for starting rtl_433, it includes the command line choices:
I recommend using:
`-M protocol -M level -C si -M time:unix:usec:utc`
at minimum.

---
Special Thanks:
CJG and MG for inspiring me with their RTL-SDR security demos, showing me how easily i can graph other people's weather instruments

Hailey Bannister and CJG for helping me learn python.

[Wunderground API](https://support.weather.com/s/article/PWS-Upload-Protocol?language=en_US)
