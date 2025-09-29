# test-hardware
repository containing working test sketches and code for modules and microcontrollers

# Tested Components
| MCU | modules |
| --- | --- |
| ESP32 | 0V7679, ILI9341, HCSR04, JSN-SR04M, DHT11, 16x2 I²C LCD, SX1278 |
| Arduino | HCSR04, JSN-SR04M, DHT11, 16x2 I²C LCD, SX1278 |
| Tang Nano 9K FPGA | SPI LCD, SX1278 |

# Notes
## compilation for different LoRa modes
`pio run -e <lora_rx or lora_tx>`

## global installation of libraries
`pio pkg install -g --library "<library name from library tab>"`

## Never forgetti
- add the pio.exe to environment variabes
- do not include angled brackets <>
