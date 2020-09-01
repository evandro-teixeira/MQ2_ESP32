/**
 * @file main.cpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 31-08-2020
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <Arduino.h>
#include "mq2.h"

mq2 sensor(36); /* GPIO36 - ADC1_0  */
float lpg = 0.0F, co = 0.00F, smoke = 0.00F;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  sensor.begin();
  Serial.printf("\n\rTesting the MQ2 Sensor ");
}

void loop() 
{
  // put your main code here, to run repeatedly:
  lpg = sensor.get(GAS_LPG) * 100.00F;
  co = sensor.get(GAS_CO) * 100.00F;
  smoke = sensor.get(GAS_SMOKE) * 100.00F;
  Serial.printf("\n\rLPG: %f \t CO: %f \t SMOKE: %f",lpg,co,smoke);
  delay(500);
}