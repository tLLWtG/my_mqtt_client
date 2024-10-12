#include "Util.h"

// 随机客户端 ID
String genClientID()
{
	String clientID = "tllwtg_";
	randomSeed(millis());
	clientID += String(random(0xffff), HEX);
	return clientID;
}

// 编码 Remaining Length
byte* encodeRemainingLength(int remaining_length)
{
	byte* ba = new byte[5];
	int index = 0;

	do
	{
		byte current_byte = remaining_length % 128;
		remaining_length /= 128;

		// 如果后面还有字节，则将当前字节的最高位设为 1
		if (remaining_length > 0)
		{
			current_byte |= 0x80;
		}

		ba[index++] = current_byte;
	} while (remaining_length > 0);

	ba[index] = '\0';
	return ba;
}

// 编码带长度信息的字符串
int encodeStringWithLen(String str, byte* byte_array, int start_index)
{
	if (str.length() > 0)
	{
		int length = str.length();

		byte_array[start_index++] = length >> 8;
		byte_array[start_index++] = length & 0xFF;

		for (int i = 0; i < length; i++)
		{
			byte_array[start_index++] = str[i];
		}
	}
	return start_index;
}
