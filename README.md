# test-hardware
repository containing working test sketches and code for modules and microcontrollers

# Tested Components
| Module/Peripheral | Arduino | ESP32 | Tang Nano 9k FPGA |
| --- | --- | --- | --- |
| 0V7679 Camera Module | ✔ | ✔ | |
| 16x2 I²C LCD | ✔ | ✔ | |
| 28BYJ-48 5v Stepper Motor + ULN2003 driver | ✔ | ✔ | |
| BME280 Environment Sensor |   | ✔ | |
| DHT11 Temperature and Humidity Sensor| ✔ | ✔ | |
| DS18B20 Temperature Sensor (probe) | ✔ | ✔ | |
| HCSR04 Ultrasonic Sensor | ✔ | ✔ | |
| ILI9341 TFT LCD |   | ✔ | |
| JSN-SR04M Ultrasonic Sensor (Waterproof) | ✔ | ✔ | |
| SEN0564 Mems CO Sensor| | ✔ | |
| SIM800L (and EVB) GSM Module | ✔ | ✔ | |
| SPI LCD |  |  | ✔ |
| SX1278 LoRa module | ✔ | ✔ |  |

## compilation for different LoRa modes
Set the envs in the `.ini` file for `lora_rx` and `lora_tx` then run `pio run -e <lora_rx or lora_tx>`

## global installation of libraries
`pio pkg install -g --library "<library name from pio library tab>"`

## Never forgetti
- add the pio.exe to environment variabes
- do not include angled brackets <>

## Extra Notes
- Some ESP32s require the `boot` button to be held while uploading sketch, attach a 10uF capacitor parallel to the `boot` button
- L CH340G 🥀, W CP2102 🙏 
