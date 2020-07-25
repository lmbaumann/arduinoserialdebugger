/**
 * SerialDebugger.ino - Example usage of SerialDebugger for Arduino.
 * Created by Luca Baumann, 2020
 */

// Serial speeds
// 9600 19200 38400 57600 74880 115200 230400 250000 500000 1000000 2000000

#include "SerialDebugger.hpp"


int testiterator = 0;
int testarray[4] = {0, 1, 2, 3};
float testarray2[4] = {0, 1, 2, 3};
unsigned long testreg = 0x01;
byte testreg2 = 0x31;

void setup()
{
    Serial.begin(2000000);
}

void loop()
{
    testiterator += 1;
    log_variable("testiterator", testiterator);
    log_array("testarray", testarray, 4);
    log_array("testarray2", testarray2, 4);
    log_register("testreg", testreg);
    log_register("testreg2", testreg2);
    breakpoint(1);
}
