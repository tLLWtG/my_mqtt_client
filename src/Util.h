#ifndef __UTIL_H
#define __UTIL_H

#include <Arduino.h>

String genClientID();

byte* encodeRemainingLength(int remaining_length);

int encodeStringWithLen(String str, byte* byte_array, int start_index);

#endif
