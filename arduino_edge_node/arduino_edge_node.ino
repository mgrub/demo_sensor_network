#include "Wire.h"
#include "SPI.h"
#include "SparkFunLSM6DS3.h"

#include "ArduinoJson.h"
#include "ArduinoMqttClient.h"
#include "WiFiNINA.h"
#include "NTPClientCustom.h"
#include "WiFiUdp.h"

// load secrets and sensor self description classes
#include "arduino_secrets.h"
#include "sensor.h"
ArduinoSecrets secrets;
SensorConfig sconf;

// print messages also on serial interface
bool verbose = false;

// init WIFI client
WiFiClient wifiClient;

// init MQTT client
MqttClient mqttClient(wifiClient);

// global variables for ntp
WiFiUDP ntpUDP;
NTPClientCustom timeClient(ntpUDP, secrets.mqtt_broker, 0, 60000); // offset, update interval

// define I2C address of accelerometer
LSM6DS3 IMU(I2C_MODE, 0x6A);

// global variables to evaluate actual output data rate
double empirical_odr = 0;
double previous_time_watermark = 0.0;
int previous_read_samples = 0;

void setup(void)
{

  // setup serial interface
  Serial.begin(57600); // start serial for output
  delay(1000);         // relax

  // setup WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(secrets.wifi_ssid);
  while (WiFi.begin(secrets.wifi_ssid, secrets.wifi_pass) != WL_CONNECTED)
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
  Serial.println(secrets.mqtt_broker);
  mqttClient.setUsernamePassword(secrets.mqtt_user, secrets.mqtt_pass);

  if (!mqttClient.connect(secrets.mqtt_broker, secrets.mqtt_port))
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
  mqttClient.beginMessage(sconf.mqtt_description_topic, true); // true -> message will be retained
  mqttClient.print(sconf.sensor_self_description);
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

  DynamicJsonDocument doc(4096);
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
  time_watermark = (double)millis() / 1000;
  time_epoch = timeClient.getEpochTime();
  doc["timestamp_epoch"] = serialized(String(time_epoch));  // to keep full double precision

  // heuristic to get proper sampling frequency / ODR
  // This is a heuristic, because difference between watermarks does not cover the same
  // interval as the counted amount of samples. However, both are periodic and quite stable.
  if (previous_read_samples)
  {
    empirical_odr = (double)previous_read_samples / (time_watermark - previous_time_watermark);
  }

  // oldest entry in buffer is (time_index * empirical_odr) old
  time_index = -IMU.settings.fifoThreshold / 3; // three axis stored in fifo
  sample_index = 0;

  // loop until FIFO is empty
  while ((IMU.fifoGetStatus() & 0x1000) == 0)
  {
    ts.add((double)time_index / empirical_odr); // time delta to time_watermark in [s]
    acc_x.add(IMU.calcAccel(IMU.fifoRead()));
    acc_y.add(IMU.calcAccel(IMU.fifoRead()));
    acc_z.add(IMU.calcAccel(IMU.fifoRead()));

    time_index++;
    sample_index++;
  }

  // send readings via MQTT
  mqttClient.beginMessage(sconf.mqtt_data_topic, (unsigned long)measureJson(doc));
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();

  // update time (only done once every update_interval milli-seconds)
  timeClient.update();

  // prepare next loop-cycle
  previous_read_samples = sample_index;
  previous_time_watermark = time_watermark;

  if (verbose)
  {
    // time + sample rate
    Serial.print(time_epoch);
    Serial.print(", empirical ODR: ");
    Serial.print(empirical_odr);
    Serial.print("\n");

    // length and content of json object
    Serial.print("Size of JSON-object: ");
    Serial.print((unsigned long)measureJson(doc));
    Serial.print("\n");
    serializeJson(doc, Serial);

    // fifo status
    Serial.print("\nFifo Status 1 and 2 (16 bits): 0x");
    Serial.println(IMU.fifoGetStatus(), HEX);
    Serial.print("\n");
  }

  // turn LED off
  digitalWrite(LED_BUILTIN, LOW);
}