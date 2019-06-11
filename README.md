# IoT-Garden-Project

This project was created as part of the requirements for my degree.

This project is designed to monitor and control facets of an indoor garden remotely.

Every hour it will take readings from several sensors and send that data to a web server. 
These sensors are reading the temperature, relative humidity, soil moisture level and light values in lux. 
A webserver will store the data in a MySQL database, so it can be retrieved later. 
The user will be able to access this data by going to the website and selecting a date. 
The site will then plot a line chart based on all the data retrieved for the chosen date.

The second part of this project is to control mechanical and electrical systems. 
These include a light, water pump, heater, extraction fan and a servo-controlled camera. 
The light and the pump can be controlled via a page on the website. 

The last part of the project will allow users to view some limited data without needing to access the webserver.
This monitoring system is using a 3.5” TFT screen with a resistive touchscreen. 
A custom designed menu system allows traversal of some basic variable data, 
such as the most recent temperature and humidity values.

If you wish to build this project yourself you will need the following hardware:

1 Raspberry Pi, being used as a LAMP server (or other external domain)

5 NodeMCU ESP8266

1 Adafruit feather Huzzah ESP8266 (optional)

1 Adafruit 3.5" 480x320 TFT FeatherWing (optional)

1 DHT22 or 11 temperature sensor

1 Capacitive soil sensor (we used DFRobot Capacitive Soil Moisture sensor)

1 BH1750FVI Digital Light Intensity Sensor

2 250vac 10A relay

2 Bipolar Junction Transistors (we used BC517 for testing)

2 Optocouplers

1 Submersible water pump (we used 12v dc for testing, but it really should be more powerful, and connected by relay)
Various components for testing (simulate mains voltage circuits with transistors first, before moving on to relays)

Note: We simulated the heater, light for safety reasons with LEDs. This way we didn't need to mess around with mains voltage.
We also simulated an extraction fan and with a 12v 120mm PC fan






