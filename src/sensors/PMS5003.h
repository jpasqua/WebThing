/*
 * PMS5003.h
 *
 * This is a mash up of Adafruit's PM25 AQI driver
 * (https://github.com/adafruit/Adafruit_PMS5003) with 
 * the SwapBap PMS driver (https://github.com/SwapBap/PMS)
 *
 * It is jsut the UART part of the Adafruit driver along
 * with the control commands of the SwapBap driver
 *
 * ORIGINAL NOTICES from Adafruit library:
 * Written by Ladyada for Adafruit Industries.
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef PMS5003_h
#define PMS5003_h

#include <Arduino.h>
#include "AQIReadings.h"

class PMS5003 {
public:
  PMS5003();

  // Prepares the hardware via the supplied serial interface (HardwareSerial/SoftwareSerial)
  bool begin(Stream* theStream);

  // Put the device to sleep. This reduces power usage and extends the life of the sensor
  void sleep();

  // Wakeup a sleeping device. Wait 30 seconds after waking for stable readings
  void wakeUp();

  // Put the device into Active mode which is the default mode after power up.
  // In this mode sensor will send serial data to the host automatically.
  void activeMode();

  // Put the device into Passive mode. In this mode sensor will send serial data
  // to the host upon request.
  void passiveMode();

  // Request a read while in Passive mode
  void requestRead();

  // Read a packet of data if one is available. Return false if no data was available or
  // if the data was bad in some way.
  bool read(AQIReadings* data);


private:
  enum DeviceMode { MODE_ACTIVE, MODE_PASSIVE };
  static constexpr uint8_t StartByteValue = 0x42;
  static constexpr uint8_t SecondByteValue = 0x4D;
  static constexpr uint8_t PacketSize = 32;
  static constexpr uint8_t FieldSize = 2;
  static constexpr uint8_t NumberOfFields = PacketSize/FieldSize;

  static constexpr uint8_t CMD_ChangeMode = 0xE1;
  static constexpr uint8_t   Mode_Passive = 0;
  static constexpr uint8_t   Mode_Active = 1;
  static constexpr uint8_t CMD_Read = 0xE2;
  static constexpr uint8_t CMD_SetAwakeAsleep = 0xE4;
  static constexpr uint8_t   GotoSleep = 0;
  static constexpr uint8_t   Wakeup = 1;

// Packet Wire Format:
// Byte   Description
// ------ Start Word 
  static constexpr uint8_t StartWordFieldIndex = 0;
// 0      Start character 1 (0x42)
// 1      Start character 2 (0x4d)
// ------ Frame Length (13 Data Values + Checksum = 13*2 + 2)
  static constexpr uint8_t FrameLengthFieldIndex = 1;
// 2      High byte of frame length
// 3      Low byte of frame length
// vvvvvv Standard Readings vvvvvv
// ------ PM1.0 Standard
  static constexpr uint8_t PM01StdFieldIndex = 2;
// 4      High byte of PM1.0 Std
// 5      Low byte of PM1.0 Std
// ------ PM2.5 Standard
  static constexpr uint8_t PM25StdFieldIndex = 3;
// 6      High byte of PM2.5 Std
// 7      Low byte of PM2.5 Std
// ------ PM10.0 Standard
  static constexpr uint8_t PM100StdFieldIndex = 4;
// 8      High byte of PM10.0 Std
// 9      Low byte of PM10.0 Std
// vvvvvv Env Readings vvvvvv
// ------ PM1.0 Env
  static constexpr uint8_t PM01EnvFieldIndex = 5;
// 10     High byte of PM1.0 Env
// 11     Low byte of PM1.0 Env
// ------ PM2.5 Env
  static constexpr uint8_t PM25EnvFieldIndex = 6;
// 12     High byte of PM2.5 Env
// 13     Low byte of PM2.5 Env
// ------ PM10.0 Env
  static constexpr uint8_t PM100EnvFieldIndex = 7;
// 14     High byte of PM10.0 Env
// 15     Low byte of PM10.0 Env
// vvvvvv Particle Count Readings vvvvvv
// ------ 0.3 um Particle Count
  static constexpr uint8_t PM03FieldIndex = 8;
// 16     High byte of 0.3 um
// 17     Low byte of 0.3 um
// ------ 0.5 um Particle Count
  static constexpr uint8_t PM05FieldIndex = 9;
// 18     High byte of 0.5 um
// 19     Low byte of 0.5 um
// ------ 1.0 um Particle Count
  static constexpr uint8_t PM10FieldIndex = 10;
// 20     High byte of 1.0 um
// 21     Low byte of 1.0 um
// ------ 2.5 um Particle Count
  static constexpr uint8_t PM25FieldIndex = 11;
// 22     High byte of 2.5 um
// 23     Low byte of 2.5 um
// ------ 5.0 um Particle Count
  static constexpr uint8_t PM50FieldIndex = 12;
// 24     High byte of 5.0 um
// 25     Low byte of 5.0 um
// ------ 10.0 um Particle Count
  static constexpr uint8_t PM100FieldIndex = 13;
// 26     High byte of 10.0 um
// 27     Low byte of 10.0 um
// vvvvvv Other vvvvvv
// ------ Reserved
  static constexpr uint8_t ReservedFieldIndex = 14;
// 28     High byte of Reserved
// 29     Low byte of Reserved
// ------ Checksum (over bytes 0-27)
  static constexpr uint8_t ChecksumFieldIndex = 15;
// 30     High byte of checksum
// 31     Low byte of checksum



  Stream*     _serialDevice = nullptr;
  DeviceMode  _mode = MODE_ACTIVE;

  bool mock;
  void fabricateData(AQIReadings* data);  // Only used when mocking
  uint16_t fieldFromPacket(uint8_t* buffer, uint8_t fieldIndex);
  void sendCommand(uint8_t command, uint8_t data);
};





#endif  // PMS5003_h