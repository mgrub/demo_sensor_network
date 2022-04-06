#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "SPI.h"

LSM6DS3 IMU(I2C_MODE, 0x6A);

// evaluation of actual output data rate
float empirical_odr = 0;
float previous_time_watermark = 0.0;
int previous_read_samples = 0;

void setup(void)
{
  // Over-ride default settings if desired
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

  // Non-basic mode settings
  IMU.settings.commMode = 1;

  // FIFO control settings
  IMU.settings.fifoThreshold = 100; // Can be 0 to 4096 (16 bit bytes)
  IMU.settings.fifoSampleRate = 50; // Hz.  Can be: 10, 25, 50, 100, 200, 400, 800, 1600, 3300, 6600
  IMU.settings.fifoModeWord = 6;    // FIFO mode.
  // FIFO mode.  Can be:
  //   0 (Bypass mode, FIFO off)
  //   1 (Stop when full)
  //   3 (Continuous during trigger)
  //   4 (Bypass until trigger)
  //   6 (Continous mode)

  Serial.begin(57600); // start serial for output
  delay(1000);         // relax...
  Serial.println("Processor came out of reset.\n");

  // Call .begin() to configure the IMUs
  if (IMU.begin() != 0)
  {
    Serial.println("Problem starting the IMU.");
  }
  else
  {
    Serial.println("IMU started.");
  }

  Serial.print("Configuring FIFO with no error checking...");
  IMU.fifoBegin();
  Serial.print("Done!\n");

  Serial.print("Clearing out the FIFO...");
  IMU.fifoClear();
  Serial.print("Done!\n");
}

void loop()
{
  int time_index;
  int sample_index;

  float time_watermark;
  float ts;
  float acc_x;
  float acc_y;
  float acc_z;

  uint16_t tempUnsigned;

  // Wait for watermark
  while ((IMU.fifoGetStatus() & 0x8000) == 0)
  {
  };

  // get time when the watermark was reached
  time_watermark = (float)millis() / 1000;

  // heuristic to get proper sampling frequency / ODR
  // This is a heuristic, because difference between watermarks does not cover the same
  // interval as the counted amount of samples. However, both are periodic and quite stable.
  if (previous_read_samples)
  {
    empirical_odr = previous_read_samples / (time_watermark - previous_time_watermark);
  }

  // Now loop until FIFO is empty.  NOTE:  As the FIFO is only 8 bits wide,
  // the channels must be synchronized to a known position for the data to align
  // properly.  Emptying the fifo is one way of doing this (this example)

  // oldest entry in buffer is i * dt old
  time_index = -IMU.settings.fifoThreshold / 3; // three axis stored in fifo
  sample_index = 0;

  Serial.print(time_watermark);
  Serial.print(", empirical ODR: ");
  Serial.print(empirical_odr);
  Serial.print("\n");

  while ((IMU.fifoGetStatus() & 0x1000) == 0)
  {

    // ts = (float)i / empirical_odr;  // time delta to time_watermark in [s]
    ts = time_watermark + (float)time_index / empirical_odr; // "absolute time" [s]
    acc_x = IMU.calcAccel(IMU.fifoRead());
    acc_y = IMU.calcAccel(IMU.fifoRead());
    acc_z = IMU.calcAccel(IMU.fifoRead());

    Serial.print(time_index);
    Serial.print(",");
    Serial.print(ts);
    Serial.print(",");
    Serial.print(acc_x);
    Serial.print(",");
    Serial.print(acc_y);
    Serial.print(",");
    Serial.print(acc_z);
    Serial.print("\n");

    time_index++;
    sample_index++;
    delay(10); // Wait for the serial buffer to clear (~50 bytes worth of time @ 57600baud)
  }

  // prepare next loop-cycle
  previous_read_samples = sample_index;
  previous_time_watermark = time_watermark;

  tempUnsigned = IMU.fifoGetStatus();
  Serial.print("\nFifo Status 1 and 2 (16 bits): 0x");
  Serial.println(tempUnsigned, HEX);
  Serial.print("\n");
}