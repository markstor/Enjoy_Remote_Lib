# CLI using CC1101 module
Example of using the remote by sending words throght the serial port.

### GPIO connections

I am using a ESP32 dev board together with a CC1101 chip in the board E07-M1101D. The connections used by default in the example are:

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

You can change some of the pins by changing the build flags in the `platform.ini` file.

### Commands

A word is sent by writing to an open monitor terminal, then pressing Enter or waiting a couple of seconds. All commands are case insensitve, except the `counter:XX` command.

| Word sent           | Command description                                                                   |
|---------------------|---------------------------------------------------------------------------------------|
| Up                  | Send "Up" button                                                                      |
| Down                | Send "Down" button                                                                    |
| Stop                | Send "Stop" button                                                                    |
| Prog                | Send "Prog" button                                                                    |
| counter:XX          | Set internal counter to the HEX number XX (replace XX by any two HEX digits, e.g. A9) |
| `anything else`     | Send "Stop" button                                                                    |

