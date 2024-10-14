#ifndef __MQTTCLIENT_HPP
#define __MQTTCLIENT_HPP

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

	bool connected()
	{
		bool res = _wifiClient.connected();
		if (res == 0)
			Serial.println("WifiClient lost connection.");
		return res;
	}

	int connect()
	{
		if (!_wifiClient.connect(_host, _port))
		{
			Serial.println("WifiClient failed to connect.");
			return 0;
		}
		Serial.println("WifiClient connected.");
		ConnectPacket packet(_client_id, _username, _password, keep_alive, will_topic,
			will_message, will_retain, will_qos, clean_session);
		sendPacket(packet);
		return 1;
	}

	void disconnect()
	{
		DisconnectPacket packet;
		sendPacket(packet);
		_wifiClient.stop();
	}

	void publish(String topic, String message, int qos = 0, bool retain = false)
	{
		PublishPacket packet(topic, message, getPacketId(), retain, qos);
		sendPacket(packet);
	}

	void reportAlive()
	{
		PingreqPacket packet;
		sendPacket(packet);
	}

	// 清空 socket 缓冲区
	void clearRecBuff()
	{
		while (_wifiClient.available())
		{
			_wifiClient.read();
		}
	}

	// 处理 socket 接收到的各种报文，若有订阅的消息，则返回解码后的消息，否则返回空 String
	String handleRec()
	{
		String res = "";
		if (_wifiClient.available() == 0)
			return res;
		byte byte1;
		_wifiClient.readBytes(&byte1, 1);
		if (!((byte1 >> 4) ^ (0b00000010))) // connack
		{
			Serial.println("Receive ConnackPacket.");
			_wifiClient.readBytes(&byte1, 1);
			_wifiClient.readBytes(&byte1, 1);
			_wifiClient.readBytes(&byte1, 1);
			if (byte1 != 0)
			{
				Serial.println("Connect Return code of connack packet is unaccepted, discoonected.");
				_wifiClient.stop();
			}
		}
		else if (!((byte1 >> 4) ^ (0b00000011))) // publish
		{
			Serial.println("Receive PublishPacket.");
			// TODO
		}
		else if (!((byte1 >> 4) ^ (0b00001001))) // suback
		{
			Serial.println("Receive SubackPacket.");
			// TODO
		}
		else if (!((byte1 >> 4) ^ (0b00001101))) // pingresp
		{
			Serial.println("Receive PingrespPacket.");
			_wifiClient.readBytes(&byte1, 1);
		}
		else
		{
			Serial.println("Received invalid packet, disconnected.");
			_wifiClient.stop();
		}
		return res;
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
		if (!_wifiClient.connected())
		{
			Serial.println("WiFiClient not connected, unable to send packet.");
			return;
		}
		int sz = 0;
		byte* packetBytes = packet.toBytes(sz);
		Serial.printf("Packet Size: %d.", sz);
		Serial.println();
		for (int i = 0; i < sz; ++i)
		{
			Serial.printf("%d ", int(packetBytes[i]));
		}
		Serial.println();
		int send_sz = _wifiClient.write(packetBytes, sz);
		if (sz != send_sz)
		{
			Serial.printf("Only sent: %d of %d.", send_sz, sz);
			Serial.println();
		}
		delete[] packetBytes;
	}
};

#endif
