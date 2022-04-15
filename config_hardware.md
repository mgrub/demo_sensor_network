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
pw: strawberry

### GPS timing

...

### NTP server

sudo apt-get install pps-tools ntp


/boot/config.txt (add the following lines)

```
# enable gps time signal (pps)
dtoverlay=pps-gpio
```

/etc/modules (add the following line)

```
pps-gpio
```

sudo reboot


/etc/ntp.conf (add?)

```
# pps-gpio /dev/pps0
server 127.127.22.0 minpoll 4 maxpoll 4
fudge 127.127.22.0 refid PPS

# gpsd clock via shm
server 127.127.28.0 minpoll 4 maxpoll 4 prefer
fudge 127.127.28.0 time1 +0.130 refid GPS flag1 1
```

### access point Wifi + NAT for internet over eth0

#### synpopsis

SSID: MQTTbrokerWifi
pw: mqtt_broker
host: 192.168.1.1

#### setup

sudo apt install dnsmasq hostapd iptables

/etc/dhcpcd.conf (add at the end)
```
interface wlan0
static ip_address=192.168.1.1/24
nohook wpa_supplicant
```

/etc/dnsmasq.conf (create/overwrite/old config saved)
```
interface=wlan0
no-dhcp-interface=eth0
dhcp-range=192.168.1.100,192.168.1.200,255.255.255.0,24h
dhcp-option=option:dns-server,192.168.1.1
```

/etc/hostapd/hostapd.conf (create/overwrite)
```
interface=wlan0
#driver=nl80211

# general config
ssid=MQTTbrokerWifi
channel=1
hw_mode=g
ieee80211n=1
ieee80211d=1
country_code=DE
wmm_enabled=1

# encryption
auth_algs=1
wpa=2
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
wpa_passphrase=mqtt_broker
```

/etc/default/hostapd (append at end)
```
RUN_DAEMON=yes
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

```
sudo systemctl unmask hostapd
sudo systemctl start hostapd
sudo systemctl enable hostapd
```

/etc/sysctl.conf (uncomment this line)
```
net.ipv4.ip_forward=1
```

```
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

/etc/rc.local (before line with "exit 0")
```
iptables-restore < /etc/iptables.ipv4.nat
```

### MQTT broker

sudo apt-get mosquitto

/etc/mosquitto/conf.d/broker.conf:
...

mosquitto_pwgen -c /etc/mosquitto/pwfile.broker default

user: default
password: mosquitto

## Arduino

### Upload Code

path: C:\Users\gruber04\Downloads\arduino-cli_0.21.1_Windows_64bit\arduino-cli.exe

compile + upload

```powershell
.\arduino-cli.exe compile -b arduino:samd:nano_33_iot .\read_fifo_testing\
.\arduino-cli.exe upload -b arduino:samd:nano_33_iot -p COM4 .\read_fifo_testing\
```

## Windows

### MQTT Explorer

### Arduino Development

arduino-cli: <https://arduino.github.io/arduino-cli/0.21/commands/arduino-cli_config_set/>

```powershell
.\arduino-cli.exe config set network.proxy https://webproxy.bs.ptb.de:8080

# Arduino Nano 33 IoT requirement
.\arduino-cli.exe core install arduino:samd

# WifiNINA, LSM6DS3 Accelerometer, MQTT, JSON libraries
.\arduino-cli.exe lib install WiFiNINA SparkFun_LSM6DS3_Breakout ArduinoMqttClient ArduinoJson
```
