#ifndef __CLIENT_H
#define __CLIENT_H

#include "Packet.hpp"
#include "Util.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

const uint16_t keep_alive = 60;
const String will_topic = "";
const String will_message = "";
const bool will_retain = false;
const uint8_t will_qos = 0;
const bool clean_session = true;

class mqttClient
{
public:
	mqttClient(const char* host, int port = 1883, String client_id = genClientID(), String username = "", String password = "")
	{
		_host = host;
		_port = port;
		_username = username;
		_password = password;
		_client_id = client_id;
		_packet_id = 0;
	}

	int connect()
	{
		if (!_wifiClient.connect(_host, _port))
		{
			Serial.println("Connection failed");
			return 0;
		}
		Serial.println("Connected");
		ConnectPacket packet(_client_id, _username, _password, keep_alive, will_topic,
			will_message, will_retain, will_qos, clean_session);
		sendPacket(packet);
		return 1;
	}

	void publish(const char* topic, const byte* message, int length, int qos = 0, bool retain = false)
	{
		
	}

	void reportAlive()
	{
		PingreqPacket packet;
		sendPacket(packet);
	}

private:
	WiFiClient _wifiClient;
	const char* _host;
	int _port;
	String _client_id;
	String _username;
	String _password;
	int _packet_id;

	int getPacketId()
	{
		_packet_id++;
		return _packet_id;
	}

	void sendPacket(Packet& packet)
	{
		int sz = 0;
		byte* packetBytes = packet.toBytes(sz);
		Serial.printf("Packet Size:%d", sz);
		Serial.println();
		for (int i = 0; i < sz; ++i)
		{
			Serial.println(int(packetBytes[i]));
		}
		Serial.println();
		_wifiClient.write(packetBytes, sz);
		delete[] packetBytes;
	}
};

#endif
