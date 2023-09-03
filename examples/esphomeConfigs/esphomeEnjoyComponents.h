#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <EnjoyRemote.h>

#include "esphome.h"

#define EMITTER_GPIO     16
#define CC1101_FREQUENCY 433.92

#define COVER_OPEN   1.0f
#define COVER_CLOSED 0.0f

static const char* TAG = "EnjoyRemote";

class EnjoyESPRemote : public Component {
private:
  bool multicast;

public:
  EnjoyRemote enjoyRemote;
  std::vector<esphome::cover::Cover*> covers;

  EnjoyESPRemote(uint32_t address)
      : Component(),
        enjoyRemote(EMITTER_GPIO, address, "mandoEnjoy") {}

  void setup() override {
    pinMode(EMITTER_GPIO, OUTPUT);
    digitalWrite(EMITTER_GPIO, LOW);
    multicast = false;

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
  }

  void sendCC1101Command(EnjoyCommand command, uint8_t blindNumber) {
    ELECHOUSE_cc1101.SetTx();
    ESP_LOGI(TAG, "Sending command %01x to blind %d. Counter: %d", command, blindNumber, this->enjoyRemote.getCurrentCode());
    enjoyRemote.sendCommand(command, blindNumber, 0, this->multicast);

    ELECHOUSE_cc1101.setSidle();
  }

  void add_cover(uint8_t blindNumber);
};

class EnjoyESPCover : public Cover {
private:
  uint8_t blindNumber;

public:
  EnjoyESPRemote* remote;
  EnjoyESPCover(EnjoyESPRemote* remote, uint8_t number)
      : Cover(),
        remote(remote),
        blindNumber(number) {}

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    traits.set_supports_stop(true);
    return traits;
  }

  void command(EnjoyCommand command) {
    this->remote->sendCC1101Command(command, this->blindNumber);
  }

  void control(const CoverCall& call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI(TAG, "OPEN");
        command(EnjoyCommand::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI(TAG, "CLOSE");
        command(EnjoyCommand::Down);
      } else {
        ESP_LOGI(TAG, "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI(TAG, "STOP");
      command(EnjoyCommand::Stop);
    }
  }

  void program() {
    ESP_LOGI(TAG, "PROG");
    command(EnjoyCommand::Prog);
  }
};

void EnjoyESPRemote::add_cover(uint8_t blindNumber) {
  auto cover = new EnjoyESPCover(this, blindNumber);
  covers.push_back(cover);
}