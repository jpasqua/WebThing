/*
 * PMS5003.h
 *
 * This is a mash up of Adafruit's PM25 AQI driver
 * (https://github.com/adafruit/Adafruit_PM25AQI) with 
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

#include "PMS5003.h"

PMS5003::PMS5003() {}

void PMS5003::sendCommand(uint8_t command, uint8_t data) {
  if (mock) return;
  uint8_t buf[7];
  uint16_t checksum = StartByteValue + SecondByteValue + command + data;
  buf[0] = StartByteValue;
  buf[1] = SecondByteValue;
  buf[2] = command;
  buf[3] = 0;
  buf[4] = data;
  buf[5] = checksum >> 8;
  buf[6] = checksum & 0xff;
  _serialDevice->write(buf, sizeof(buf));
}

void PMS5003::sleep() { sendCommand(CMD_SetAwakeAsleep, GotoSleep); }

void PMS5003::wakeUp() { sendCommand(CMD_SetAwakeAsleep, Wakeup); }

void PMS5003::activeMode() {
  sendCommand(CMD_ChangeMode, Mode_Active);
  _mode = MODE_ACTIVE;
}

void PMS5003::passiveMode() {
  sendCommand(CMD_ChangeMode, Mode_Passive);
  _mode = MODE_PASSIVE;
}

void PMS5003::requestRead() {
  if (_mode == MODE_PASSIVE) sendCommand(CMD_Read, 0);
}

bool PMS5003::begin(Stream *theSerial) {
  _serialDevice = theSerial;
  mock = (theSerial == nullptr);
  return true;
}

uint16_t PMS5003::fieldFromPacket(uint8_t* buffer, uint8_t fieldIndex) {
  uint16_t field;
  field  = (buffer[fieldIndex * 2 + 1]);    // Low byte comes second
  field += (buffer[fieldIndex * 2] << 8);   // High byte comes first
  return field;
}

bool PMS5003::read(AQIReadings *data) {
  if (mock) {
    fabricateData(data);
    return true;
  }

  uint8_t buffer[PacketSize];
  uint16_t sum = 0;

  if (!data) return false;
  if (!_serialDevice->available()) return false;

  if (_serialDevice->peek() != StartByteValue) {
    int p;
    while ( ((p = _serialDevice->peek()) != -1) && (p != StartByteValue) ) { _serialDevice->read(); }
    if (p == -1) return false;
  }

  // Assuming all of the data is there, read it in...
  if (_serialDevice->available() < PacketSize) return false;
  _serialDevice->readBytes(buffer, PacketSize);

  // Calculate checksum over all fields (except the checksum itself)
  for (uint8_t i = 0; i < PacketSize-FieldSize; i++) {
    sum += buffer[i];
  }

  // Serial.print("Raw buffer: ");
  // for (uint8_t i = 0; i < PacketSize-FieldSize; i++) {
  //   if (i) Serial.print(", ");
  //   Serial.print(buffer[i], HEX); 
  // }
  // Serial.println();

  // The data on the wire is big endian. Make sure it works for this platform.
  data->standard.pm10 = fieldFromPacket(buffer, PM01StdFieldIndex); 
  data->standard.pm25 = fieldFromPacket(buffer, PM25StdFieldIndex); 
  data->standard.pm100 = fieldFromPacket(buffer, PM100StdFieldIndex); 
  data->env.pm10 = fieldFromPacket(buffer, PM01EnvFieldIndex); 
  data->env.pm25 = fieldFromPacket(buffer, PM25EnvFieldIndex); 
  data->env.pm100 = fieldFromPacket(buffer, PM100EnvFieldIndex); 
  data->particles_03um = fieldFromPacket(buffer, PM03FieldIndex); 
  data->particles_05um = fieldFromPacket(buffer, PM05FieldIndex); 
  data->particles_10um = fieldFromPacket(buffer, PM10FieldIndex); 
  data->particles_25um = fieldFromPacket(buffer, PM25FieldIndex); 
  data->particles_50um = fieldFromPacket(buffer, PM50FieldIndex); 
  data->particles_100um = fieldFromPacket(buffer, PM100FieldIndex); 

  uint16_t checksum = fieldFromPacket(buffer, ChecksumFieldIndex);
  if (sum != checksum) {
    Serial.println("PMS5003 Checksum did not match!");
    return false;
  }

  // Sanity check on the data
  if (data->particles_03um + data->particles_05um + data->particles_10um + 
      data->particles_25um + data->particles_50um + data->particles_100um == 0) {
    Serial.println("PMS5003 Sanity Check: All particle level fields are Zero!");
    return false;
  }

  Serial.println("PMS5003 successfully took a reading");
  return true;
}

void PMS5003::fabricateData(AQIReadings* data) {
  static AQIReadings mockReadings;
  static bool initialized = false;

  if (!initialized) {
    mockReadings.standard.pm10 = random(0, 30);
    mockReadings.particles_03um = random(10, 42);
    initialized = true;
  } else {
    int low = std::max(-3, -((int)mockReadings.standard.pm10));
    int hi = std::min(50-mockReadings.standard.pm10, 4);
    mockReadings.standard.pm10 += random(low, hi);
    low = std::max(-3, -((int)mockReadings.particles_03um));
    hi = std::min(75-mockReadings.particles_03um, 4);
    mockReadings.particles_03um += random(low, hi);
  }

  mockReadings.standard.pm25 = mockReadings.standard.pm10 + random(0, 3);
  mockReadings.standard.pm100 = mockReadings.standard.pm25 + random(0, 3);
  mockReadings.env.pm10 = mockReadings.standard.pm10 + random(0, 3);
  mockReadings.env.pm25 = mockReadings.env.pm10  + random(0, 3);
  mockReadings.env.pm100 = mockReadings.env.pm25  + random(0, 3);
  
  mockReadings.particles_05um = mockReadings.particles_03um/2;
  mockReadings.particles_10um = mockReadings.particles_05um/2;
  mockReadings.particles_25um = mockReadings.particles_10um/2;
  mockReadings.particles_50um = mockReadings.particles_25um/2;
  mockReadings.particles_100um = mockReadings.particles_50um/2;

  *data = mockReadings;
}
