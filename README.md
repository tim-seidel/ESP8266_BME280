# ESP8266_BME280

This is a small weather station project. It shows weather data on a display and can send it to MQTT broker if desired. This can be choosen on the device when you start it.

The hardware is: 
 - BME280
 - SSD1306
 - ESP8266 (D1 mini)

## BME280

This a sensor from Bosch it can read:
 - Temperature
 - Humidity
 - Air Pressure

## ESP8266

This a micro controller from espressif. It is used for:
 - Reading the BME280 via I2C
 - Displaying the information on a SSD1306 oled display 
 - Connecting to a local network (optional)
 - Sending the measure data to a mqtt broker (optional)

## SSD1306
 
This is a small oled display. It is used to display:
 - WIFI and MQTT status information 
 - Weather data (Temperature, Humidity, Air Pressure). Due to the display being so small the data is being interated and each displayed on its own.

# Installation

To use this project you need:
 - The visual studio code extension "PlatformIO"
 - A config file "config.h" located in the "include/" folder that contains
 ```
    #define WIFI_SSID "YourSSID"
    #define WIFI_PWD "Your PWD"
    #define MQTT_SERVER_ADDR "MQTT Broker address you are using "
 ```
 - Don't forget that the WIFI/MQTT part is optional and only used if you select it via a button press at the start of the device. So if you have no MQTT broker, you can still run it.

 ## Libraries used
  
This project uses:
 - (PlatformIO)
 - PubSubClient
 - Adafruit BME280 Library
 - Adafruit SSD1306
 - Thanks for that =)
