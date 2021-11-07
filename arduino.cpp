#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Arduino_MKRIoTCarrier.h>
#include "config.h"
#include <ArduinoJson.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h

int status = WL_IDLE_STATUS; // the WiFi radio's status

int rep = 0;
int rep2 = 0;

MKRIoTCarrier carrier;
WiFiClient wificlient;
PubSubClient client(wificlient);

uint32_t red = carrier.leds.Color(0, 255, 0);
uint32_t green = carrier.leds.Color(255, 0, 0);
uint32_t yellow = carrier.leds.Color(219, 255, 0);

int disTemp, disHumi, disPres, curDis;

DynamicJsonDocument doc(1024);

void connectToWiFi()
{
  status = WiFi.begin(SSID);
  delay(5000);
  if (status != WL_CONNECTED)
  {
    Serial.println("Couldn't connect to WiFi...");
    carrier.leds.fill(red, 0, 5);
    //carrier.leds.show();
  }
  else
  {
    Serial.println("Connected to WiFi!");
    Serial.println("\nStarting connection...");
    carrier.leds.fill(green, 0, 2);
    carrier.leds.fill(green, 3, 2);
    carrier.leds.setPixelColor(2, 219, 255, 0);
    //carrier.leds.show();
  }
}
void reconnectMQTTClient()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(CLIENT_NAME.c_str()))
    {
      Serial.println("connected");
      carrier.leds.setPixelColor(2, 255, 0, 0);
      //carrier.leds.show();
    }
    else
    {
      Serial.print("Retrying in 3 seconds - failed, rc=");
      Serial.println(client.state());
      carrier.leds.setPixelColor(2, 0, 255, 0);
      //carrier.leds.show();
      carrier.display.fillScreen(ST77XX_BLACK);
      carrier.display.setTextColor(ST77XX_RED);
      carrier.display.setTextSize(18);
      carrier.display.setCursor(80, 55);
      carrier.display.print("!");
      delay(1000);
      carrier.leds.setPixelColor(2, 0, 0, 0);
      //carrier.leds.show();
      delay(1000);
      carrier.leds.setPixelColor(2, 0, 255, 0);
      //carrier.leds.show();
      delay(1000);
      carrier.leds.setPixelColor(2, 0, 0, 0);
      //carrier.leds.show();
      connectToWiFi();
    }
  }
}

void createMQTTClient()
{
  client.setServer(BROKER.c_str(), 1883);
  reconnectMQTTClient();
}

void displayTemp(int temperature)
{
  disTemp = temperature;
  curDis = 0;
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setTextColor(ST77XX_GREEN);
  carrier.display.setTextSize(4);

  carrier.display.setCursor(80, 54);
  carrier.display.print("Temp");
  carrier.display.setCursor(60, 110);
  carrier.display.setTextSize(7);
  carrier.display.print(temperature);
  carrier.display.print("C");
}

void displayHumi(int humidity)
{
  disHumi = humidity;
  curDis = 1;
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setTextColor(ST77XX_GREEN);
  carrier.display.setTextSize(4);

  carrier.display.setCursor(80, 54);
  carrier.display.print("Humi");
  carrier.display.setCursor(60, 110);
  carrier.display.setTextSize(7);
  carrier.display.print(humidity);
  carrier.display.print("%");
}

void displayPres(int pressure)
{
  disPres = pressure;
  curDis = 2;
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setTextColor(ST77XX_GREEN);
  carrier.display.setTextSize(4);

  carrier.display.setCursor(80, 54);
  carrier.display.print("Pres");
  carrier.display.setCursor(60, 110);
  carrier.display.setTextSize(7);
  carrier.display.print(pressure);
}

void setup()
{
  Serial.begin(9600);
  rep++;
  CARRIER_CASE = true;
  carrier.begin();
  carrier.display.setRotation(0);
  carrier.leds.setBrightness(35);

  carrier.leds.fill(yellow, 0, 5);
  //carrier.leds.show();
  connectToWiFi();
  createMQTTClient();
}

void postMessage(string topic, string message)
{
  string telemetry = message;
  const string CLIENT_TELEMETRY_TOPIC = topic;

  client.publish(CLIENT_TELEMETRY_TOPIC.c_str(), telemetry.c_str());
}

void loop()
{
  reconnectMQTTClient();
  rep++;
  rep2++;
  int temperature = carrier.Env.readTemperature();
  int humidity = carrier.Env.readHumidity();
  int pressure = carrier.Pressure.readPressure();
  carrier.Buttons.update();
  Serial.println(rep2);
  if (rep2 == 30)
  {
    switch (curDis)
    {
    case 0:
      if (disTemp != temperature)
      {
        displayTemp(temperature);
      }
      break;
    case 1:
      if (disHumi != humidity)
      {
        displayHumi(humidity);
      }
      break;
    case 2:
      if (disPres != pressure)
      {
        displayPres(pressure);
      }
      break;
    case 3:

      break;
    }
    rep2 = 0;
  }
  if (carrier.Buttons.onTouchDown(TOUCH0))
  {
    displayTemp(temperature);
  }
  if (carrier.Buttons.onTouchDown(TOUCH1))
  {
    displayHumi(humidity);
  }
  if (carrier.Buttons.onTouchDown(TOUCH2))
  {
    displayPres(pressure);
  }
  if (carrier.Buttons.onTouchDown(TOUCH3))
  {

  }

  if (rep == 75)
  {
    //doc["temp"] = temperature;
    doc["humi"] = humidity;
    //doc["pres"] = pressure;
    string message;
    serializeJson(doc, message);
    postMessage("arduino", message);
    rep = 0;
  }
}
