#include <DaikinTHCHeatpumpIR.h>

DaikinTHCHeatpumpIR::DaikinTHCHeatpumpIR() : HeatpumpIR()
{
  static const char model[] PROGMEM = "daikinthc";
  static const char info[]  PROGMEM = "{\"mdl\":\"daikinthc\",\"dnthc\":\"DaikinTHC\",\"mT\":18,\"xT\":30,\"fs\":6,\"maint\":[10,11,12,13,14,15,16,17]}}}";

  _model = model;
  _info = info;
}


// Daikin numeric values to command bytes
void DaikinTHCHeatpumpIR::send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd)
{
  // Sensible defaults for the heat pump mode

  uint8_t operatingMode = DAIKIN_AIRCON_MODE_OFF | DAIKIN_AIRCON_MODE_AUTO;
  uint8_t fanSpeed      = DAIKIN_AIRCON_FAN_AUTO;
  uint8_t temperature   = 23;

  switch (powerModeCmd)
  {
    case POWER_ON:
      operatingMode |= DAIKIN_AIRCON_MODE_ON;
      break;
  }

  switch (operatingModeCmd)
  {
    case MODE_AUTO:
      operatingMode |= DAIKIN_AIRCON_MODE_AUTO;
      break;
    case MODE_HEAT:
      operatingMode |= DAIKIN_AIRCON_MODE_HEAT;
      break;
    case MODE_COOL:
      operatingMode |= DAIKIN_AIRCON_MODE_COOL;
      break;
    case MODE_DRY:
      operatingMode |= DAIKIN_AIRCON_MODE_DRY;
      temperatureCmd = 0x24;
      break;
    case MODE_FAN:
      operatingMode |= DAIKIN_AIRCON_MODE_FAN;
      temperatureCmd = 0xC0;
      break;
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = DAIKIN_AIRCON_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = DAIKIN_AIRCON_FAN1;
      break;
    case FAN_2:
      fanSpeed = DAIKIN_AIRCON_FAN2;
      break;
    case FAN_3:
      fanSpeed = DAIKIN_AIRCON_FAN3;
      break;
    case FAN_4:
      fanSpeed = DAIKIN_AIRCON_FAN4;
      break;
    case FAN_5:
      fanSpeed = DAIKIN_AIRCON_FAN5;
      break;
  }

  if ((operatingModeCmd == MODE_HEAT && temperatureCmd >= 10 && temperatureCmd <= 30) ||
      (temperatureCmd >= 18 && temperatureCmd <= 30))
  {
    temperature = temperatureCmd * 2 ;
  }

  sendDaikinTHC(IR, operatingMode, fanSpeed, temperature, swingVCmd, swingHCmd);
}


// Send the Daikin code
void DaikinTHCHeatpumpIR::sendDaikinTHC(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH)
{
//   (void)swingV;
//   (void)swingH;

  uint8_t daikinTHCTemplate[19] = {
    0x11, 0xDA, 0x27, 0x00, 0x00, // First header
   //  0     1     2     3     4  
    0x31, 0x32, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x00, 0x08, 0xE2}; // Second header, this seems to have the wall clock time (bytes 12 & 13 are changing)
   //  5     6     7     8     9     10   11    12    13    14    15    16    17    18    
  if (operatingMode & DAIKIN_AIRCON_MODE_ON) {
    daikinTHCTemplate[16] = 0x00; // Power On
  } else {
    daikinTHCTemplate[16] = 0x40; // Power Off
  }

  daikinTHCTemplate[5] = operatingMode;
  daikinTHCTemplate[6] = temperature;
  daikinTHCTemplate[8] = fanSpeed;
  Serial.print("swingv");Serial.println(swingV,HEX);
  daikinTHCTemplate[8] |= swingV;
  // Checksum calculation
  // * Checksums at bytes 7 and 15 are calculated the same way
  uint8_t checksum = 0x00;

  for (int i=0; i<18; i++) {
    checksum += daikinTHCTemplate[i];
  }

  daikinTHCTemplate[18] = checksum &= 0xFF;

  // 38 kHz PWM frequency
  IR.setFrequency(38);

  // Header
  IR.mark(DAIKIN_AIRCON_HDR_MARK);
  IR.space(DAIKIN_AIRCON_HDR_SPACE);

  // First header
  for (int i=0; i<5; i++) {
    IR.sendIRbyte(daikinTHCTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

//   // Pause + new header
//   IR.mark(DAIKIN_AIRCON_BIT_MARK);
//   IR.space(DAIKIN_AIRCON_MSG_SPACE);
//   IR.mark(DAIKIN_AIRCON_HDR_MARK);
//   IR.space(DAIKIN_AIRCON_HDR_SPACE);

//   // Second header - this probably has the wall clock time
//   for (int i=5; i<14;i++) {
//     IR.sendIRbyte(daikinTHCTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
//   }

  // Pause + new header
  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(DAIKIN_AIRCON_MSG_SPACE);
  
    // Serial.println("Sending byte:");
    // for(int i=0;i<19;i++){
    //     if(daikinTHCTemplate[i]<0x10) Serial.print("0");
    //     Serial.print(daikinTHCTemplate[i], HEX);
    //     Serial.print(",");
    // }
    // Serial.println();
  // Last 19 bytes - the actual payload
  for (int i=5; i<19; i++) {
    IR.sendIRbyte(daikinTHCTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  IR.mark(DAIKIN_AIRCON_BIT_MARK);
  IR.space(0);
}