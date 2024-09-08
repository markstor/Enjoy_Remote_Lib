#include <EnjoyRemote.h>

#define ENJOY_SYMBOL_US 418

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
  byte frame[8];
  buildFrame(frame, command, rollingCode, selectedBlind, multicast);
  sendFrame(frame, true);
  for (int i = 0; i < repeat; i++) {
    sendLow(26800);
    sendFrame(frame, false);
  }
}

void EnjoyRemote::printFrame(byte* frame) {
  for (byte i = 0; i < 8; i++) {
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
  frame[0] = 0; // Always 0
  if (multicast) {
    frame[1] = 0x0f;
  } else {
    frame[1] = 0x1 << selectedBlind; // Channels Mask
  }
  frame[2] = (button << 4) | ((remoteId & 0xf00) >> 8); // Which button did  you press? The last 4 bits are the first 4 bits of the remote address
  frame[3] = remoteId & 0xff; // remaining address of the remote
  frame[4] = 0; // No idea, reserved for more addresses maybe?
  frame[5] = selectedBlind; // Selected Blind, either 0, 1, 2 or 3

  int xor_value = (frame[2] & 0xf0) | (frame[5] ^ 0x01);
  frame[6] = xor_value ^ code; // Remote address

  // remainder calculation: CRC8 with 0x80 polynomial
  uint8_t remainder = 0x00;
  unsigned byte, bit;
  for (byte = 0; byte < 7; ++byte) {
    remainder ^= frame[byte];
    for (bit = 0; bit < 8; ++bit) {
      if (remainder & 0x80) {
        remainder = (remainder << 1) ^ 0x07;
      } else {
        remainder = (remainder << 1);
      }
    }
  }

  frame[7] = remainder; // CRC8 of previous bytes

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

  // Hardware sync: 64 syncs for the first frame, 24 for the following ones.
  sendHigh(370);
  sendLow(2500);
  for (int i = 0; i < nSyncs; i++) {
    sendLow(400);
    sendHigh(400);
  }

  // Software sync
  sendLow(4000);
  sendHigh(400);
  sendLow(400);

  // Data: bits are sent one by one, starting with the MSB.
  // With G.E. Thomas convention of Manchester Encoding (0 is a transition from low to high)
  for (byte i = 0; i < 64; i++) {
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
