#include "Wire.h"
#include "SPI.h"
#include "SparkFunLSM6DS3.h"

#include "ArduinoJson.h"
#include "ArduinoMqttClient.h"
#include "WiFiNINA.h"
#include "NTPClientCustom.h"
#include "WiFiUdp.h"

#include "arduino_secrets.h"
#include "sensor_1.h"

// global variables for wifi
char wifi_ssid[] = WIFI_SSID;
char wifi_pass[] = WIFI_PASS;
WiFiClient wifiClient;

// global variables for mqtt
char mqtt_user[] = MQTT_USER;
char mqtt_pass[] = MQTT_PASS;
const char mqtt_broker[] = MQTT_BROKER;
int mqtt_port = MQTT_PORT;
const char mqtt_data_topic[] = MQTT_DATA_TOPIC;
const char mqtt_description_topic[] = MQTT_DESCRIPTION_TOPIC;
const char sensor_self_description[] = SENSOR_SELF_DESCRIPTION;
MqttClient mqttClient(wifiClient);

// global variables for ntp
WiFiUDP ntpUDP;
NTPClientCustom timeClient(ntpUDP, MQTT_BROKER, 0, 60000); // offset, update interval

// define I2C address of accelerometer
LSM6DS3 IMU(I2C_MODE, 0x6A);

// global variables to evaluate actual output data rate
float empirical_odr = 0;
double previous_time_watermark = 0.0;
int previous_read_samples = 0;

void setup(void)
{

  // setup serial interface
  Serial.begin(57600); // start serial for output
  delay(1000);         // relax

  // setup WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(wifi_ssid);
  while (WiFi.begin(wifi_ssid, wifi_pass) != WL_CONNECTED)
  {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected.\n\n");

  // setup time synchronization
  timeClient.begin();
  previous_time_watermark = timeClient.getEpochTime();

  // setup MQTT
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqtt_broker);
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);

  if (!mqttClient.connect(mqtt_broker, mqtt_port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }
  Serial.println("Connected.\n\n");

  // setup IMU + FIFO
  IMU.settings.gyroEnabled = 0;     // Can be 0 or 1
  IMU.settings.gyroFifoEnabled = 0; // Set to include gyro in FIFO

  IMU.settings.accelEnabled = 1;
  IMU.settings.accelRange = 4;                  // Max G force readable.  Can be: 2, 4, 8, 16
  IMU.settings.accelSampleRate = 13;            // Hz.  Can be: 13, 26, 52, 104, 208, 416, 833, 1666, 3332, 6664, 13330
  empirical_odr = IMU.settings.accelSampleRate; // best guess at the start
  IMU.settings.accelBandWidth = 400;            // Hz.  Can be: 50, 100, 200, 400;
  IMU.settings.accelFifoEnabled = 1;            // Set to include accelerometer in the FIFO
  IMU.settings.accelFifoDecimation = 1;         // set 1 for on /1
  IMU.settings.tempEnabled = 1;

  IMU.settings.commMode = 1; // Non-basic mode settings

  IMU.settings.fifoThreshold = 100; // Can be 0 to 4096 (16 bit bytes)
  IMU.settings.fifoSampleRate = 50; // Hz.  Can be: 10, 25, 50, 100, 200, 400, 800, 1600, 3300, 6600
  IMU.settings.fifoModeWord = 6;    // FIFO mode: 0 off, 1 stop when full, 3 continuous during trigger, 4 bypass until trigger, 6 continouos

  if (IMU.begin() != 0)
  {
    Serial.println("Problem starting the IMU.");
  }
  IMU.fifoBegin(); // start the FIFO
  IMU.fifoClear(); // clear to sync all registers

  // broadcast self description
  mqttClient.beginMessage(mqtt_description_topic, true); // true -> message will be retained
  mqttClient.print(sensor_self_description);
  mqttClient.endMessage();

  // finish setup
  Serial.print("Setup done!\n");
}

void loop()
{
  int time_index;
  int sample_index;

  double time_watermark;
  double time_epoch;


  DynamicJsonDocument doc(2048);
  JsonArray ts = doc.createNestedArray("delta_ts");
  JsonArray acc_x = doc.createNestedArray("acc_x");
  JsonArray acc_y = doc.createNestedArray("acc_y");
  JsonArray acc_z = doc.createNestedArray("acc_z");

  // Wait for watermark
  while ((IMU.fifoGetStatus() & 0x8000) == 0)
  {
  };

  // turn LED on
  digitalWrite(LED_BUILTIN, HIGH);

  // get time when the watermark was reached
  time_watermark = (float)millis() / 1000;
  time_epoch = timeClient.getEpochTime(); 
  doc["timestamp_epoch"] = time_epoch;

  // heuristic to get proper sampling frequency / ODR
  // This is a heuristic, because difference between watermarks does not cover the same
  // interval as the counted amount of samples. However, both are periodic and quite stable.
  if (previous_read_samples)
  {
    empirical_odr = previous_read_samples / (time_watermark - previous_time_watermark);
  }

  // oldest entry in buffer is (time_index * empirical_odr) old
  time_index = -IMU.settings.fifoThreshold / 3; // three axis stored in fifo
  sample_index = 0;

  Serial.print(time_epoch);
  Serial.print(", empirical ODR: ");
  Serial.print(empirical_odr);
  Serial.print("\n");

  // loop until FIFO is empty
  while ((IMU.fifoGetStatus() & 0x1000) == 0)
  {
    ts.add((float)time_index / empirical_odr); // time delta to time_watermark in [s]
    acc_x.add(IMU.calcAccel(IMU.fifoRead()));
    acc_y.add(IMU.calcAccel(IMU.fifoRead()));
    acc_z.add(IMU.calcAccel(IMU.fifoRead()));

    time_index++;
    sample_index++;
  }

  // print readings
  Serial.print("Size of JSON-object: ");
  Serial.print((unsigned long)measureJson(doc));
  Serial.print("\n");
  serializeJson(doc, Serial);

  // send readings via MQTT
  mqttClient.beginMessage(mqtt_data_topic, (unsigned long)measureJson(doc));
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();

  // update time (only done once every update_interval milli-seconds)
  timeClient.update();

  // prepare next loop-cycle
  previous_read_samples = sample_index;
  previous_time_watermark = time_watermark;

  Serial.print("\nFifo Status 1 and 2 (16 bits): 0x");
  Serial.println(IMU.fifoGetStatus(), HEX);
  Serial.print("\n");
  digitalWrite(LED_BUILTIN, LOW);
}