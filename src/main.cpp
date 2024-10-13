#include <Arduino.h>
#include <ArduinoJson.h>
#include "MQTTClient.hpp"
#include "WiFiInterface.h"

void handleWiFiConnection();
static bool connecting_wifi = false;
static bool socket_status = false;

mqttClient* client;

int publish_cnt = 0;

void setup()
{
	Serial.begin(115200);
	Serial.println();
	Serial.println("setup");

	handleWiFiConnection();
	if (WiFi.status() == WL_CONNECTED)
	{
		client = new mqttClient("IP", 1883);
		if (client->connect())
			socket_status = true;
	}
	Serial.println("setup done");
}

void loop()
{
	handleWiFiConnection();
	if (WiFi.status() == WL_CONNECTED && socket_status)
	{
		client->reportAlive();
		delay(500);
		JsonDocument doc;
		doc["msg"] = "This is No." + String(++publish_cnt) + " PublishPacket.";
		String msg;
		serializeJson(doc, msg);
		client->publish("tllwtg_test", msg);
		delay(10000);
	}
}

void handleWiFiConnection()
{
	if (WiFi.status() == WL_CONNECTED)
	{
		if (connecting_wifi)
		{
			connecting_wifi = false;
			delay(300);
		}
	}
	else
	{
		if (connecting_wifi)
		{
			checkDNS_HTTP();
			checkConnect();
			delay(50);
		}
		else
		{
			connectToWiFi();
			connecting_wifi = true;
		}
	}
}
