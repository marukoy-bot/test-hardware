# test-hardware
repository containing working test sketches and code for modules and microcontrollers

# Tested Components
| Module/Peripheral | Arduino | Espressif | Tang Nano 9k FPGA | RP2040 |
| --- | :---: | :---: | :---: | :---: |
| 0V7679 Camera Module |✔|✔| | |
| 16x2 I²C LCD |✔|✔| | |
| 28BYJ-48 5v Stepper Motor + ULN2003 driver |✔|✔| | |
| A0221AT Ultrasonic Sensor (Waterproof, Controlled) |✔|✔| | |
| BME280 Environment Sensor | |✔| | |
| DHT11 Temperature and Humidity Sensor|✔|✔| | |
| DS18B20 Temperature Sensor (probe) |✔|✔| | |
| HCSR04 Ultrasonic Sensor |✔|✔| | |
| ILI9341 TFT LCD | |✔| | |
| JSN-SR04M Ultrasonic Sensor (Waterproof) |✔|✔| | |
| SEN0564 Mems CO Sensor| |✔| | |
| SIM800L (and EVB) GSM Module |✔|✔| | |
| SPI LCD | | |✔| |
| SX1278 LoRa module |✔|✔|✔| |

# Compilation for different LoRa modes
Set the envs in the `.ini` file for `lora_rx` and `lora_tx` then run `pio run -e <lora_rx or lora_tx>`

# Global installation of libraries
- `pio pkg install -g --library "<library name from pio library tab>"`
or
- `pio pkg install -g --library "<git url>"`

# Custom `debug.h` header file
Create a **debug** folder inside the library folder and copy-paste the `debug.h` header file inside
- **PlatformIO:** `C:\Users\<username>\.platformio\lib`
- **Arduino IDE:** `C:\Users\<username>\Documents\Arduino\libraries`

Add this build flag in the `.ini` file, setting the **DDEBUG** value to `1` enables debugging, while `0` disables it
`build_flags = -DDEBUG=<value>`

# Never forgetti
- add the `pio.exe` to environment variabes
- **do not** include angled brackets <>

# Extra Notes
- Some ESP32s require the **boot** button to be held while uploading sketch, attach a 10uF capacitor parallel to the **boot** button
- L CH340G 🥀, W CP2102 🙏 
