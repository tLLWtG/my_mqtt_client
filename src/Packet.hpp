#ifndef __PACKET_HPP
#define __PACKET_HPP

#include "Arduino.h"
#include "Util.h"

class Packet
{
public:
	Packet()
	{}

	virtual byte* toBytes(int& length) = 0;
};

class PublishPacket : public Packet
{
public:
	PublishPacket(String topic, String message, int packet_id, bool retain = 0, int qos = 0, bool dup = false)
		: _dup(dup)
		, _qos(qos)
		, _retain(retain)
		, _topic(topic)
		, _packet_id(packet_id)
		, _message(message)
	{}

	byte* toBytes(int& length)
	{
		byte* byte_array = new byte[256];
		int index = 0;

		// 报文类型 + dup + qos + retain
		byte_array[index++] = 0b00110000 | (int(_dup) << 3) | (int(_qos) << 1) | (int(_retain));

		// remaining_length
		byte* remaining_length_bytes = _calRemainingLengthBytes();
		for (int i = 0; remaining_length_bytes[i] != '\0'; i++)
		{
			byte_array[index++] = remaining_length_bytes[i];
		}
		delete[] remaining_length_bytes;

		// topic
		// packet id (QoS = 1 or 2)
		byte* topic_bytes = new byte[256];
		int sz = encodeStringWithLen(_topic, topic_bytes, 0);
		for (int i = 0; i < sz; ++i)
			byte_array[index++] = topic_bytes[i];
		delete[] topic_bytes;
		// 载荷
		for (int i = 0; i < _message.length(); ++i)
			byte_array[index++] = _message[i];
		length = index;
		return byte_array;
	}

	byte* _calRemainingLengthBytes()
	{
		int remaining_length = 2 + _topic.length() + _message.length();
		return encodeRemainingLength(remaining_length);
	}

private:
	bool _dup;
	int _qos;
	bool _retain;
	String _topic;
	int _packet_id;
	String _message;
};

class ConnectPacket : public Packet
{
public:
	ConnectPacket(String client_id, String username = "", String password = "", uint16_t keep_alive = 60,
		String will_topic = "", String will_message = "", bool will_retain = false, uint8_t will_qos = 0,
		bool clean_session_flag = true)
		: _client_id(client_id)
		, _username(username)
		, _password(password)
		, _keep_alive(keep_alive)
		, _will_topic(will_topic)
		, _will_message(will_message)
		, _will_retain(will_retain)
		, _will_qos(will_qos)
		, _clean_session_flag(clean_session_flag)
	{}

	byte* toBytes(int& length)
	{

		byte* byte_array = new byte[256];
		int index = 0;

		// 0x10 表示 CONNECT 报文
		byte_array[index++] = 0x10;

		byte* remaining_length_bytes = _calRemainingLengthBytes();
		for (int i = 0; remaining_length_bytes[i] != '\0'; i++)
		{
			byte_array[index++] = remaining_length_bytes[i];
		}
		delete[] remaining_length_bytes;

		byte_array[index++] = 0;
		byte_array[index++] = 4;
		byte_array[index++] = 'M';
		byte_array[index++] = 'Q';
		byte_array[index++] = 'T';
		byte_array[index++] = 'T';

		// MQTT v3.1.1 的协议等级：4
		byte_array[index++] = 4;

		// 标志位
		byte flags_byte = _calFlagsByte();
		byte_array[index++] = flags_byte;

		// 心跳
		byte* keep_alive_bytes = _calKeepAliveBytes();
		byte_array[index++] = keep_alive_bytes[0];
		byte_array[index++] = keep_alive_bytes[1];
		delete[] keep_alive_bytes;

		// 载荷
		int payload_len = 0;
		byte* payload_bytes = _calPayloadBytes(payload_len);
		for (int i = 0; i < payload_len; i++)
		{
			byte_array[index++] = payload_bytes[i];
		}
		delete[] payload_bytes;

		length = index;
		return byte_array;
	}

private:
	String _client_id;
	String _username;
	String _password;
	uint16_t _keep_alive;
	String _will_topic;
	String _will_message;
	bool _will_retain;
	uint8_t _will_qos;
	bool _clean_session_flag;

	// 计算 Remaining Length 字节
	byte* _calRemainingLengthBytes()
	{
		int remaining_length = 10; // 固定长度
		if (_client_id.length() > 0)
			remaining_length += 2 + _client_id.length();
		if (_will_topic.length() > 0)
			remaining_length += 2 + _will_topic.length();
		if (_will_message.length() > 0)
			remaining_length += 2 + _will_message.length();
		if (_username.length() > 0)
			remaining_length += 2 + _username.length();
		if (_password.length() > 0)
			remaining_length += 2 + _password.length();

		return encodeRemainingLength(remaining_length);
	}

	// 计算标志位字节
	byte _calFlagsByte()
	{
		byte flags = 0;
		if (_username.length() > 0)
			flags |= 0b10000000;
		if (_password.length() > 0)
			flags |= 0b01000000;
		if (_will_retain)
			flags |= 0b00100000;
		if (_will_topic.length() > 0 && _will_message.length() > 0)
			flags |= (_will_qos << 3) | 0b00000100;
		if (_clean_session_flag)
			flags |= 0b00000010;
		return flags;
	}

	// 计算心跳字节
	byte* _calKeepAliveBytes()
	{
		byte* keep_alive_bytes = new byte[2];
		keep_alive_bytes[0] = _keep_alive >> 8;
		keep_alive_bytes[1] = _keep_alive & 0xFF;
		return keep_alive_bytes;
	}

	// 计算载荷字节
	byte* _calPayloadBytes(int& Payload_len)
	{
		byte* bs = new byte[256];
		int index = 0;
		Serial.print("client_id: ");
		Serial.println(_client_id);
		index = encodeStringWithLen(_client_id, bs, index);
		index = encodeStringWithLen(_will_topic, bs, index);
		index = encodeStringWithLen(_will_message, bs, index);
		index = encodeStringWithLen(_username, bs, index);
		index = encodeStringWithLen(_password, bs, index);
		bs[index] = '\0';
		Payload_len = index;
		return bs;
	}
};

class PingreqPacket : public Packet
{
public:
	PingreqPacket()
	{}

	byte* toBytes(int& length)
	{
		length = 2;
		byte* byte_array = new byte[2];
		byte_array[0] = 0b11000000;
		byte_array[1] = 0b00000000;
		return byte_array;
	}
};

class DisconnectPacket : public Packet
{
public:
	DisconnectPacket()
	{}

	byte* toBytes(int& length)
	{
		length = 2;
		byte* byte_array = new byte[2];
		byte_array[0] = 0b11100000;
		byte_array[1] = 0b00000000;
		return byte_array;
	}
};

#endif
