#pragma once

#include <Arduino.h>
#include <Preferences.h>

enum class Command : byte {
  Up = 0x8,
  Stop = 0x4,
  Down = 0x2,
  Prog = 0x1,
};

Preferences enjoyPreferences;

class EnjoyRemote {
private:
  byte emitterPin;
  uint16_t remoteId;
  const char* rollingCodeStorageKey;

  uint8_t nextCode();

  void buildFrame(byte* frame, Command command, uint8_t code, uint8_t selectedBlind, bool multicast);
  void sendFrame(byte* frame, bool first);
  void printFrame(byte* frame);

  void sendHigh(uint16_t durationInMicroseconds);
  void sendLow(uint16_t durationInMicroseconds);

public:
  EnjoyRemote(byte emitterPin, uint32_t remote, const char* rollingCodeStorageKey);
  void setup();
  void setCode(uint8_t code);
  /**
	 * Send a command with this EnjoyRemote.
	 *
	 * @param command the command to send
	 * @param repeat the number how often the command should be repeated, default 0. Should
	 * 				 only be used when simulating holding a button.
	 */
  void sendCommand(Command command, uint8_t selectedBlind, int repeat = 0, bool multicast = false);
};

Command getEnjoyCommand(const String& string);