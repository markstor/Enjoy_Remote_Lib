#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <EnjoyRemote.h>

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

void sendCC1101Command(EnjoyCommand command) {
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
      //if the user writes "counter:0x40", the counter will be set to 0x40
      enjoyRemote.setCode(strtol(string.substring(8).c_str(), 0, 16));
    } else {
      // #if the user writes "up", the Up command will be sent
      const EnjoyCommand command = getEnjoyCommand(string);
      digitalWrite(LED_GPIO, HIGH);
      sendCC1101Command(command);
      digitalWrite(LED_GPIO, LOW);
    }
#ifdef DEBUG
    Serial.println("finished sending");
#endif
  }
}