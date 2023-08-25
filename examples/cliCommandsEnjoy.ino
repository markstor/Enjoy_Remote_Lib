#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <EnjoyRemote.h>

#define EMITTER_GPIO     4
#define LED_GPIO         2
#define EEPROM_ADDRESS   0
#define REMOTE_ADDRESS   0x106
#define SELECTED_BLIND   3
#define CC1101_FREQUENCY 433.92

EnjoyRemote enjoyRemote(EMITTER_GPIO, REMOTE_ADDRESS, "mando1");

void setup() {
  Serial.begin(115200);
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);
  enjoyRemote.setup();

  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
}

void sendCC1101Command(EnjoyRemote::Command command) {
  ELECHOUSE_cc1101.SetTx();
  enjoyRemote.sendCommand(command, SELECTED_BLIND);
  ELECHOUSE_cc1101.setSidle();
}

void loop() {
  if (Serial.available() > 0) {
    const String string = Serial.readStringUntil('\n');
#ifdef DEBUG
    Serial.printf("Read command: %s \n", string);
#endif
    if (string.startsWith("counter:")) {
      enjoyRemote.setCode(strtol(string.substring(8).c_str(), 0, 16));
    } else {
      const Command command = getEnjoyCommand(string);
      digitalWrite(LED_GPIO, HIGH);
      sendCC1101Command(command);
      digitalWrite(LED_GPIO, LOW);
    }
#ifdef DEBUG
    Serial.println("finished sending");
#endif
  }
}