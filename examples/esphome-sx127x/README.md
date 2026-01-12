# ESPHome SX127x Example for TTGO T3 LoRa32

ESPHome configuration for Enjoy Motors remote control using the TTGO T3 LoRa32 V1.6.1 board with SX1276/SX1278 433MHz transceiver.

## Hardware

- **Board**: LILYGO T3 LoRa32 V1.6.1 (433MHz variant)
- **RF Chip**: SX1276/SX1278
- **Display**: 0.96" SSD1306 OLED

**Important**: Always attach a 433MHz antenna before transmitting. Never transmit without an antenna.

## Pin Configuration

All pins are internally connected on the TTGO T3 board - no external wiring needed.

| Function | GPIO |
|----------|------|
| LoRa CS | 18 |
| LoRa RST | 23 |
| LoRa DIO0 | 26 |
| LoRa DIO2 | 32 |
| OLED SDA | 21 |
| OLED SCL | 22 |
| OLED RST | 16 |

## Setup

1. Copy `secrets.yaml.template` to `secrets.yaml` and fill in your values:
   ```bash
   cp secrets.yaml.template secrets.yaml
   ```

2. Edit `secrets.yaml` with your WiFi credentials, API key, and remote address:
   ```yaml
   wifi_ssid: "your_wifi_ssid"
   wifi_password: "your_wifi_password"
   api_key: "your_api_encryption_key"
   remote_address: "0x123"  # Your Enjoy remote address
   ```

3. Compile and flash:
   ```bash
   esphome compile ttgo_enjoy.yaml
   esphome run ttgo_enjoy.yaml
   ```

## Requirements

- ESPHome 2025.7 or later (native sx127x support)
- Home Assistant (for cover control)

## Home Assistant Integration

After flashing, the device will appear in Home Assistant with:

- **4 Cover entities**: Enjoy Blind 0-3 (Up/Down/Stop)
- **4 Button entities**: Enjoy Prog Blind 0-3 (for pairing)

## Pairing a New Blind

1. On your existing Enjoy Motors remote, select the blind channel
2. Press the PROG button 8 times on the original remote
3. The blind should respond with a short movement
4. Within 10 seconds, press the corresponding "Enjoy Prog Blind X" button in Home Assistant
5. The blind should move again to confirm pairing
6. Send a Stop command to confirm

## Technical Notes

This configuration uses ESPHome's documented approach for SX127x OOK continuous mode:
- `sx127x` component in OOK mode with `packet_mode: false`
- `remote_transmitter` component drives DIO2 using ESP32's RMT peripheral
- Mode switching via `on_transmit`/`on_complete` automations

### Tuning

If transmission is unreliable:
- Increase `pa_power` (max 17 for BOOST)
- Try different `bitrate` values (10000, 20000, 32000)
- Add `repeat_count: 1` or `2` to cover actions for multiple bursts
