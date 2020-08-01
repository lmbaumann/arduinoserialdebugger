/**
 * SerialDebugger.ino - Example usage of SerialDebugger for Arduino.
 * Created by Luca Baumann, 2020
 */

// Serial speeds
// 9600 19200 38400 57600 74880 115200 230400 250000 500000 1000000 2000000

#include "SerialDebugger.hpp"


int testiterator = 0;
int testarray_int[4] = {0, 1, 2, 3};
float testarray_float[4] = {0, 1, 2, 3};
unsigned long testreg_long = 0x02020202;
unsigned int testreg_int = 0x0202;
byte testreg_byte = 0x21;

void setup()
{
    Serial.begin(1000000);
}

void loop()
{
    testiterator += 1;
    testiterator %= 1000;
    log_variable("testiterator", testiterator);
    log_array("testarray_int", testarray_int, 4);
    log_array("testarray_float", testarray_float, 4);
    log_register("testreg_byte", testreg_byte);
    log_register("testreg_int", testreg_int);
    log_register("testreg_long", testreg_long);
}
