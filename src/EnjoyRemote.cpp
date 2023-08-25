#include <EnjoyRemote.h>

#define ENJOY_SYMBOL_US 408

Preferences enjoyPreferences;

EnjoyRemote::EnjoyRemote(byte emitterPin, uint32_t remoteId, const char* rollingCodeStorageKey)
    : emitterPin(emitterPin), remoteId(remoteId), rollingCodeStorageKey(rollingCodeStorageKey) {}

void EnjoyRemote::setup() {
  pinMode(emitterPin, OUTPUT);
  digitalWrite(emitterPin, LOW);
}

uint8_t EnjoyRemote::nextCode() {
  enjoyPreferences.begin("EnjoyCodes", false);
  uint8_t newValue = enjoyPreferences.getUChar(this->rollingCodeStorageKey, 0) + 1;
  enjoyPreferences.putUChar(this->rollingCodeStorageKey, newValue);
  enjoyPreferences.end();
  return newValue;
}

void EnjoyRemote::setCode(uint8_t code) {
  enjoyPreferences.begin("EnjoyCodes", false);
  enjoyPreferences.putUChar(this->rollingCodeStorageKey, code);
  enjoyPreferences.end();
}

uint8_t EnjoyRemote::getCurrentCode() {
  enjoyPreferences.begin("EnjoyCodes", false);
  uint8_t currentValue = enjoyPreferences.getUChar(this->rollingCodeStorageKey, 0);
  enjoyPreferences.end();
  return currentValue;
}

void EnjoyRemote::sendCommand(EnjoyCommand command, uint8_t selectedBlind, int repeat, bool multicast) {
  const uint8_t rollingCode = this->nextCode();
  byte frame[7];
  buildFrame(frame, command, rollingCode, selectedBlind, multicast);
  sendFrame(frame, true);
  for (int i = 0; i < repeat; i++) {
    sendFrame(frame, 7);
  }
}

void EnjoyRemote::printFrame(byte* frame) {
  for (byte i = 0; i < 7; i++) {
    if (frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      Serial.print("0"); // nibble is a 0.
    }
    Serial.print(frame[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void EnjoyRemote::buildFrame(byte* frame, EnjoyCommand command, uint8_t code, uint8_t selectedBlind, bool multicast) {
  const byte button = static_cast<byte>(command);
  if (multicast) {
    frame[0] = 0x0f;
  } else {
    frame[0] = 0x1 << selectedBlind; // Channels Mask
  }
  frame[1] = (button << 4) | ((remoteId & 0xf00) >> 8); // Which button did  you press? The last 4 bits are the first 4 bits of the remote address
  frame[2] = remoteId & 0xff; // remaining address of the remote
  frame[3] = 0; // No idea, reserved for more addresses maybe?
  frame[4] = selectedBlind; // Selected Blind, either 0, 1, 2 or 3

  int xor_value = (frame[1] & 0xf0) | (frame[4] ^ 0x01);
  frame[5] = xor_value ^ code; // Remote address

  // remainder calculation: CRC8 with 0x80 polynomial
  uint8_t remainder = 0xff;
  unsigned byte, bit;

  Serial.printf("fxor: 0x%2x\n", 0xff ^ frame[0]);

  for (byte = 0; byte < 7; ++byte) {
    remainder ^= frame[byte];
    Serial.printf("preremainder: 0x%2x\n", remainder);
    for (bit = 0; bit < 8; ++bit) {
      if (remainder & 0x80) {
        remainder = (remainder << 1) ^ 0x80;
      } else {
        remainder = (remainder << 1);
      }
    }
    Serial.printf("remainder:    0x%2x\n", remainder);
  }

  frame[6] = remainder; // CRC8 of previous bytes

#ifdef DEBUG
  Serial.printf("Counter: 0x%2x\n", code);
  Serial.print("Frame         : ");
  printFrame(frame);
#endif
}

void EnjoyRemote::sendFrame(byte* frame, bool first) {
  int nSyncs = 24;
  if (first) { // The syncs are longer only with the first frame.
    nSyncs = 64;
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < nSyncs; i++) {
    sendHigh(ENJOY_SYMBOL_US);
    sendLow(ENJOY_SYMBOL_US);
  }

  sendLow(3546);
  // Software sync
  sendHigh(ENJOY_SYMBOL_US);
  sendLow(830);
  sendHigh(ENJOY_SYMBOL_US);

  for (byte i = 0; i < 7; i++) {
    sendLow(ENJOY_SYMBOL_US);
    sendHigh(ENJOY_SYMBOL_US);
  }

  // Data: bits are sent one by one, starting with the MSB.
  // With G.E. Thomas Manchester Encoding
  for (byte i = 0; i < 56; i++) {
    if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 0) {
      sendLow(ENJOY_SYMBOL_US);
      sendHigh(ENJOY_SYMBOL_US);
    } else {
      sendHigh(ENJOY_SYMBOL_US);
      sendLow(ENJOY_SYMBOL_US);
    }
  }
}

void EnjoyRemote::sendHigh(uint16_t durationInMicroseconds) {
  digitalWrite(emitterPin, HIGH);
  delayMicroseconds(durationInMicroseconds);
}

void EnjoyRemote::sendLow(uint16_t durationInMicroseconds) {
  digitalWrite(emitterPin, LOW);
  delayMicroseconds(durationInMicroseconds);
}

EnjoyCommand getEnjoyCommand(const String& string) {
  if (string.equalsIgnoreCase("Up")) {
    return EnjoyCommand::Up;
  } else if (string.equalsIgnoreCase("Stop")) {
    return EnjoyCommand::Stop;
  } else if (string.equalsIgnoreCase("Down")) {
    return EnjoyCommand::Down;
  } else if (string.equalsIgnoreCase("Prog")) {
    return EnjoyCommand::Prog;
  } else {
    return EnjoyCommand::Stop;
  }
}
