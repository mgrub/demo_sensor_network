# Ideas for the Arduino Sensor Network

## Mechanical Setup

- 3x Arduino Nano 33 IoT
- mounted on a Y-shaped (wooden?) structure in the same orientation

## Communication Setup

### Windows

- connected to broker via wifi (via usb wifi dongle)
- runs uncertainty service?
- adapter to AgentMet4FoF?

### Arduino

- connected to broker via wifi (via internal chipset)
- each device published to MQTT topic
  - <sensor_name>/data -> binary datastream / eclipse unide v3?
  - <sensor_name>/description -> json-serialized RDF triples (retain message)

### Raspberry Pi

- provides ad-hoc wifi (via internal chipset)
- runs MQTT broker (mosquitto)
- receives time via GPS
- runs NTP server

## Links

- Broker: <http://mqtt-explorer.com/>
- Visualization: <https://mosquitto.org/download/>
- MQTT Docs: <https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-mqtt-device-to-device>
- MQTT Tutorial: <https://community.element14.com/challenges-projects/design-challenges/design-for-a-cause-2021/b/blog/posts/connecting-the-arduino-nano-33-iot-with-local-mqtt-broker-2>
- Acceleration Sensor Docs: <https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf> (page 36 -> FIFO mode)
- Acceleration Sensor Library: <https://github.com/STMicroelectronics/STMems_Standard_C_drivers/tree/master/lsm6ds3_STdC/examples>
- NTP Library: <https://www.arduino.cc/reference/en/libraries/ntpclient/>
- Adhoc Wifi: <https://www.elektronik-kompendium.de/sites/raspberry-pi/2002171.htm>
- GPS USB NTP: <https://klenzel.de/4182>
- GPS receiver doc: <https://www.waveshare.com/wiki/NEO-M8T_GNSS_TIMING_HAT>

## Open Questions

- absolute timestamps
- max. achievable sample rate / transfer rate
