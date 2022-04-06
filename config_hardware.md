# Configuration Manual

## Raspberry Pi

### Operating System

Raspberry Pi OS Lite
<https://downloads.raspberrypi.org/raspios_arm64/images/raspios_arm64-2022-01-28/2022-01-28-raspios-bullseye-arm64.zip>

-> copy to microSD card, prepare ssh, prepare wifi

default packages: git ssh vim python3

### Account

host: mqttbroker
user: pi
pw: digi

### GPS timing

...

### NTP server

...

### Adhoc Wifi

...

### MQTT broker

sudo apt-get mosquitto

## Arduino

### Upload Code

...

## Windows

### MQTT Explorer

### Arduino Development

arduino-cli: <https://arduino.github.io/arduino-cli/0.21/commands/arduino-cli_config_set/>

```powershell
.\arduino-cli.exe config set network.proxy https://webproxy.bs.ptb.de:8080

# Arduino Nano 33 IoT requirement
.\arduino-cli.exe core install arduino:samd

# WifiNINA, LSM6DS3 Accelerometer, MQTT libraries
.\arduino-cli.exe lib install WiFiNINA Arduino_LSM6DS3 ArduinoMqttClient
```