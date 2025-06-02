# Enjoy Remote Lib
Emulate an Enjoy Motors remote using an ESP32 with a 433MHz transmitter.

This project is an Arduino Library able to emulate an Enjoy Motors remote control on ESP32 devices.
It was entirely developed by reverse engineering an EMSTEEL4, I have not tested it on any other scenario.
Because it is a Arduino Library it can easily included in existing projects or used standalone.

### Hardware Requirements

For this library to work you need an 433.92 Mhz RF transmitter. I have only tested it with a CC1101 module, I recommned it because there is a lot of other projects that use it, and it can be tuned to other frequencies as well.
For the CC1101 module the [SmartRC-CC1101-Driver-Lib](https://github.com/LSatan/SmartRC-CC1101-Driver-Lib) must be installed.

### Usage

I have written two use cases for the library:
1. A "command line interface" (CLI) to send commands by writing to the serial port, here on the same repo: [cliCC1101](examples/cliCC1101/)
2. An ESPHome configuration to easily integrate it on Home Assistant, available on another repo: [esphome-configs](https://github.com/markstor/esphome-configs/)

I am using a ESP32 dev board together with a CC1101 chip in the board E07-M1101D. The connections used by default in all the examples are:

| ESP32 board | CC1101 module (E07-M1101D) |
|-------------|----------------------------|
| 3.3V        | 2 (Vcc)                    |
| GND         | 1 (GND)                    |
| D4          | 8 (GD02)                   |
| D16         | 3 (GD00)                   |
| D5          | 4 (CKN)                    |
| D18         | 5 (SCK)                    |
| D19         | 7 (MISO)                   |
| D23         | 6 (MOSI)                   |

#### Rolling Code Storage

This library stores the rolling code in the Non-Volatile Storage (NVS) of the ESP32, using the `Preferences.h` library
The rolling number is an uint8 that increases by 1 at any command sent.
Then, this number is XORed by some info of the frame sent.

I have not written yet a complete description of the protocol, but the source code of the [buildFrame](https://github.com/markstor/Enjoy_Remote_Lib/blob/main/src/EnjoyRemote.cpp#L57) and [sendFrame](https://github.com/markstor/Enjoy_Remote_Lib/blob/main/src/EnjoyRemote.cpp#L95) functions should be enough to follow it.

#### Register the remote

Before the emulated remote can control the devices, the remote must be registered.
Therefore you can refer to the original manual of your Enjoy Motors device.
My recommended way is to use an already registered remote for this. See page 4 of the [manual](https://enjoy-motors.com/instrucciones/EMSTEEL4-8-16_esp_online.pdf). Here you have a summary:
1. Select, on your orignal remote, the blind you want to register
2. Press 8 times the PROG button of your original remote, the blind should respond with a short move.
3. Within 10 seconds, send the PROG command from your module. Ideally with the same number defined as selected blind (take into account that you have to substract 1 from the number shown in the original remote display, i.e. blind 1 in the remote display is blind 0 in the protocol). The blind should move again.
4. Send the Stop command from your module.
