#include <Arduino.h>

bool decodeDaikinTHC(byte *bytes, int byteCount)
{
  // If this looks like a Daikin code...
  if ( byteCount == 19 && bytes[0] == 0x11 && bytes[1] == 0xDA && bytes[2] == 0x27 ) {
    Serial.println(F("Looks like a Daikin protocol"));

    // Power mode
    switch (bytes[5] & 0x01) {
      case 0x00:
        Serial.println(F("POWER OFF"));
        break;
      case 0x01:
        Serial.println(F("POWER ON"));
        break;
    }

    // Operating mode
    switch (bytes[5] & 0x70) {
      case 0x00:
        Serial.println(F("MODE AUTO"));
        break;
      case 0x30:
        Serial.println(F("MODE COOL"));
        break;
      case 0x20:
        Serial.println(F("MODE DRY"));
        break;
      case 0x60:
        Serial.println(F("MODE FAN"));
        break;
      }

    // Temperature
    Serial.print(F("Temperature: "));
    Serial.println(bytes[6] / 2);

    // Fan speed
    switch (bytes[8] & 0xF0) {
      case 0xB0:
        // Outdoor unit quiet
        Serial.println(F("FAN: QUIET"));
        break;
      case 0xA0:
        Serial.println(F("FAN: AUTO"));
        break;
      case 0x30:
        Serial.println(F("FAN: 1"));
        break;
      case 0x40:
        Serial.println(F("FAN: 2"));
        break;
      case 0x50:
        Serial.println(F("FAN: 3"));
        break;
      case 0x60:
        Serial.println(F("FAN: 4"));
        break;
      case 0x70:
        Serial.println(F("FAN: 5"));
        break;
    }
    // Other flags
    Serial.print(F("FLAGS: "));
    if (bytes[13] & 0x01) Serial.print(F("POWERFUL "));
    if (bytes[8] & 0xF == 0x0F) Serial.print(F("SWING ON"));
    else Serial.print(F("SWING OFF"));
    // if (bytes[32] & 0x04) Serial.print(F("ECONO "));
    // if (bytes[24] & 0x0F) Serial.print(F("SWING "));
    // if (bytes[29] & 0x20) Serial.print(F("QUIET"));
    Serial.println();

    // Check if the checksum matches
    byte checksum = 0x00;
    for (byte i =0;i<18;i++){
      checksum += bytes[i];
    }
    checksum &= 0xFF;
    Serial.print("checksum read : "); Serial.println(checksum, HEX);
    if ( bytes[18] == checksum ) {
      Serial.println(F("Checksum matches"));
    } else {
      Serial.println(F("Checksum does not match"));
    }
    return true;
  }

  return false;
}
