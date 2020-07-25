/**
 * SerialDebugger.hpp - Simple serial debugger for Arduino.
 * Created by Luca Baumann, 2020
 */

#ifndef SERIALDEBUGGER_H
#define SERIALDEBUGGER_H

// Defines which serial port to use
#define DEBUGGER_SERIAL Serial

#include <Arduino.h>

/**
 * Add a breakpoint to the code. Wait for serial command to continue.
 * name: optional name for the breakpoint
 * id: optional id for breakpoint
 */
void breakpoint();
void breakpoint(String name);
void breakpoint(int id);

/**
 * General log
 */
void log(String text);

/**
 * Log a variable.
 * name: variable name to be logged
 * variable: value to be logged
 */
void log_variable(String name, int variable);
void log_variable(String name, int* variable, int len);
void log_variable(String name, unsigned int* variable, int len);
void log_variable(String name, long* variable, int len);
void log_variable(String name, unsigned long* variable, int len);
void log_variable(String name, float* variable, int len);
void log_variable(String name, char* variable, int len);

#endif