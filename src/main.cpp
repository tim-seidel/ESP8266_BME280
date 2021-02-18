#include <Arduino.h>
#include "Wire.h"
#include <Adafruit_I2CDevice.h>
#include "Adafruit_BME280.h"
#include "Adafruit_SSD1306.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "../include/config.h"

#define SENSOR_RATE 5000  // Local display update rate
#define UPDATE_RATE 60000 // Mqtt update rate
#define MAX_PAYLOAD 60    // Mqtt payload length

#define DEBUG false       // Serial print

Adafruit_BME280 bmeSensor;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

WiFiClient espClient;
PubSubClient client(espClient);

int pinButton = D3;

bool withWifi = true;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PWD;
const char *mqtt_server = MQTT_SERVER_ADDR;

const char *hostname = "ESP8266";
const char *clientId = "ESP8266";
const char *topic_data = "/esp8266/bme280";
char msg[MAX_PAYLOAD];

struct SensorData
{
  float temp;
  float humidity;
  float pressure;
};
SensorData current;

void print(String msg)
{
#if DEBUG
  Serial.print(msg);
#endif
}

void println(String msg)
{
#if DEBUG
  Serial.println(msg);
#endif
}

void toString(SensorData &data, char *buffer)
{
  sprintf(buffer, "{\"temp\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}", data.temp, data.humidity, data.pressure);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  println(topic);
}

void reset_display()
{
  display.clearDisplay();
  display.setCursor(0, 0);
}

void display_msg(String msg)
{
  reset_display();
  display.setTextSize(2);
  display.print(msg);
  display.display();
}

bool isButtonPressed()
{
  return LOW == digitalRead(pinButton);
}

void setup_bme()
{
  if (!bmeSensor.begin(0x76))
  {
    println("Couldnt find BME Sensor");
  }
  bmeSensor.setSampling(
      Adafruit_BME280::MODE_FORCED,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::FILTER_OFF);

  current.temp = bmeSensor.readTemperature();
  current.humidity = bmeSensor.readHumidity();
  current.pressure = bmeSensor.readPressure() / 100.0;
}

void setup_display()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  reset_display();
  display.setRotation(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void setup_wifi()
{
  display_msg("Wifi setup");
  print("Connecting to ");
  print(ssid);
  print(" ");

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    print(".");
  }

  print("\nWiFi connected. IP address: ");
  println(WiFi.localIP().toString());
  display_msg("Connected:\n" + WiFi.localIP().toString());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_button()
{
  pinMode(pinButton, INPUT);
}

bool setup_ask_for_wifi()
{
  uint16_t decision_time = 10000;
  uint16_t poll_time = 50;
  float remaining = decision_time / 1000.0;

  display_msg("WLAN an?");
  for (uint16_t i = 0; i < decision_time / poll_time; i++)
  {
    if (isButtonPressed())
    {
      return true;
    }
    delay(poll_time);
    remaining = (decision_time - poll_time * (i + 1)) / 1000.0;
    display_msg("WLAN an?\nStart in:\n" + String(remaining));
  }
  return false;
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  setup_button();
  setup_bme();
  setup_display();

  withWifi = setup_ask_for_wifi();
  if (withWifi)
  {
    setup_wifi();
  }
  else
  {
    display_msg("Starte...\nOhne WLAN");
  }

  delay(SENSOR_RATE);
}

void display_temp(SensorData &data, bool hasWifi)
{
  reset_display();
  display.setTextSize(2);
  display.println("Temp [C]:");
  display.println(hasWifi ? "---" : "--");
  display.setTextSize(3);
  display.println(data.temp);
  display.display();
}

void display_hum(SensorData &data, bool hasWifi)
{
  reset_display();
  display.setTextSize(2);
  display.println("Hum [%]:");
  display.println(hasWifi ? "---" : "--");
  display.setTextSize(3);
  display.println(data.humidity);
  display.display();
}

void display_pressure(SensorData &data, bool hasWifi)
{
  reset_display();
  display.setTextSize(2);
  display.println("Pre [hPa]:");
  display.println(hasWifi ? "---" : "--");
  display.setTextSize(3);
  display.println(data.pressure);
  display.display();
}

void publish_data()
{
  toString(current, msg);
  client.publish(topic_data, msg);
}

void read_data()
{
  bmeSensor.takeForcedMeasurement();
  current.temp = bmeSensor.readTemperature();
  current.humidity = bmeSensor.readHumidity();
  current.pressure = bmeSensor.readPressure() / 100.0;
}

void loop()
{
  for (int i = 0; i < UPDATE_RATE / (SENSOR_RATE * 3); i++)
  {
    bool hasWifi = !WiFi.isConnected();
    read_data();

    delay(SENSOR_RATE);
    display_temp(current, hasWifi);

    delay(SENSOR_RATE);
    display_hum(current, hasWifi);

    delay(SENSOR_RATE);
    display_pressure(current, hasWifi);
  }

  if (withWifi)
  {
    if (client.connected())
    {
      client.loop();
      publish_data();
    }
    else
    {
      if (client.connect(clientId))
      {
        println("Connection to client successful.");
        client.loop();
        publish_data();
      }
      else
      {
        display_msg("No mqtt.");
        print("Couldn't connect to MQTT server: ");
        println(String(client.state()));
      }
    }
  }
}