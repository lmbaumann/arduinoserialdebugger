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

void setup()
{
    Serial.begin(2000000);
}

void loop()
{
    testiterator += 1;
    log_variable("testiterator", testiterator);
    log_variable("testarray", testarray, 4);
    log_variable("testarray", testarray2, 4);
    breakpoint(1);
}
