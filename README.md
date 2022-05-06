# Sensor Network Demonstrator

This is the repository for a small sensor network demonstrator.
It can be used to demonstrate various aspects of the IoT like edge device/fog/cloud devices, data streams, sensor self-description, homogeneous/heterogeneous networks, absolute time synchronization and wireless sensor networks.
To do so, it builds on existing standards (WIFI, MQTT, NTP, GPS, JSON) and uses off-the-shelf hardware.

The repository documents:

- hardware setup
- communication setup
- Arduino microcontroller code
- Arduino programming environment
- RaspberryPi setup

## Hardware Setup

- 3x Arduino Nano 33 IoT
- 1x RaspberryPi 4
- 1x Waveshare NEO-M8T GNSS TIMING HAT
- power supply and mount for the Arduinos
- power supply, case and fan for the RaspberryPi
- 1x development PC

## Communication Setup

```ascii
┌───────────┐  ┌───────────┐  ┌───────────┐
│ Arduino 1 │  │ Arduino 2 │  │ Arduino 3 │
│           │  │           │  │           │
│   - Wifi  │  │   - Wifi  │  │   - Wifi  │
└────┬──────┘  └────┬──────┘  └────┬──────┘
     │ ▲            │ ▲            │ ▲
 MQTT│ │        MQTT│ │        MQTT│ │
     │ │NTP         │ │NTP         │ │NTP
     ▼ │            ▼ │            ▼ │
┌──────┴──────────────┴──────────────┴────┐
│ Raspberry Pi                            │
│                                         │  Serial    ┌─────────────────┐
│   - Wifi Accesspoint                    │◄───────────┤ GNSS satellites │
│   - MQTT Broker                         │  Interface └─────────────────┘
│   - NTP Server                          │
└─────────┬───────────────────────────────┘
          │                 ▲
          │                 │
      MQTT│             MQTT│
          │                 │
          ▼                 ▼
      ┌───────┐     ┌────────────────┐
      │ Users │     │ Cloud-Services │
      └───────┘     └────────────────┘
```

### MQTT topics

- each Arduino publishes to multiple MQTT topics
  - <sensor_name>/data -> json
  - <sensor_name>/description -> json 

## Useful Resources

- MQTT Visualizer: <http://mqtt-explorer.com/>
- MQTT Broker: <https://mosquitto.org/download/>
- MQTT Docs: <https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-mqtt-device-to-device>
- MQTT Tutorial: <https://community.element14.com/challenges-projects/design-challenges/design-for-a-cause-2021/b/blog/posts/connecting-the-arduino-nano-33-iot-with-local-mqtt-broker-2>

- Acceleration Sensor Docs: <https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf> (page 36 -> FIFO mode)
- SparkFun Acceleration Sensor Library: <https://github.com/sparkfun/SparkFun_LSM6DS3_Arduino_Library>

- NTP Library: <https://www.arduino.cc/reference/en/libraries/ntpclient/>
- Adhoc Wifi: <https://www.elektronik-kompendium.de/sites/raspberry-pi/2002171.htm>
- GPS USB NTP: <https://klenzel.de/4182>
- GPS receiver doc: <https://www.waveshare.com/wiki/NEO-M8T_GNSS_TIMING_HAT>

## TODO / Future Ideas

- MQTT agent for AgentMet4FoF -> check out <https://github.com/PTB-M4D/thementag-demo>
- test high sampling rates (500-1000 Hz)
- adjust messages sent by Arduino (e.g. follow BMBF FAMOUS approach of eclipse unide)
- adjust self-description message (to something more semantic like JSON-RDF-triples, currently only {"id": "sensor_1"})
- include uncertainty service to add uncertainty based on self-description