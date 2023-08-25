#include <EnjoyRemote.h>

#include "esphome.h"

#define EMITTER_GPIO   4
#define CC1101_FREQUENCY 433.42

#define COVER_OPEN   1.0f
#define COVER_CLOSED 0.0f

class EnjoyESPCover : public Cover {
private:
  EnjoyESPRemote remote;
  uint8_t blindNumber;

public:
  EnjoyESPCover(EnjoyESPRemote remote, uint8_t number)
      : Cover() {
    this->remote = remote;
    this->blindNumber = number;
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void command(EnjoyCommand command) {
    return this->remote->sendCC1101Command(command, this->blindNumber)
  }

  void control(const CoverCall& call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI("enjoy", "OPEN");
        command(EnjoyCommand::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI("enjoy", "CLOSE");
        command(EnjoyCommand::Down);
      } else {
        ESP_LOGI("enjoy", "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI("enjoy", "STOP");
      command(EnjoyCommand::Stop);
    }
  }

  void program() {
    ESP_LOGI("enjoy", "PROG");
    command(EnjoyCommand::Prog);
  }
};

class EnjoyESPRemote : public Component {
private:
  EnjoyRemote enjoyRemote;
  bool multicast;

public:
  std::vector<esphome::cover::Cover*> covers;
  EnjoyESPRemote(uint32_t address) : Component() {
    multicast = false;
    enjoyRemote = EnjoyRemote(EMITTER_GPIO, address, "mandoUniversalEnjoy");
  }

  void setup() override {
    pinMode(EMITTER_GPIO, OUTPUT);
    digitalWrite(EMITTER_GPIO, LOW);

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
  }

  void sendCC1101Command(EnjoyCommand command, uint8_t blindNumber) {
    ELECHOUSE_cc1101.SetTx();
    enjoyRemote.sendCommand(command, blindNumber, 0, this->multicast);
    ELECHOUSE_cc1101.setSidle();
  }

  void add_cover(uint8_t blindNumber) {
    auto cover = new EnjoyESPCover(this, blindNumber);
    covers.push_back(cover);
  }
};