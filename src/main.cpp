#include <Arduino.h>
#include <ArduinoJson.h>
#include "MQTTClient.hpp"
#include "WiFiInterface.h"

void handleWiFiConnection();
static bool connecting_wifi = false;

mqttClient* client;

uint32_t publish_cnt = 0, send_cnt = 0;

void setup()
{
	Serial.begin(115200);
	Serial.println();
	Serial.println("setup.");

	handleWiFiConnection();
	if (WiFi.status() == WL_CONNECTED)
	{
		client = new mqttClient("IP", 1883);
		client->connect();
	}
	Serial.println("setup done.");
}

void loop()
{
	handleWiFiConnection();

	if (WiFi.status() == WL_CONNECTED)
	{
		if (client->connected())
		{
			if (++send_cnt >= 100)
			{
				send_cnt = 0;
				client->reportAlive();
				delay(100);
				client->clearRecBuff();
				JsonDocument doc;
				String msg;
				doc["msg"] = "This is No." + String(++publish_cnt) + " PublishPacket.";
				serializeJson(doc, msg);
				client->publish("tllwtg_test", msg);
			}
			// String rec = client->handleRec();
			// if (!rec.isEmpty())
			// {
			// 	Serial.println("Receive msg:");
			// 	Serial.println(rec);
			// }
		}
		else
		{
			client->connect();
		}
		delay(100);
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
