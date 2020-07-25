/**
 * SerialDebugger.cpp - Simple serial debugger for Arduino.
 * Created by Luca Baumann, 2020
 */

#include "SerialDebugger.hpp"

#define DEBUGGER_FLOAT_DECIMALS 5


// small enum to store data types
enum DebuggerVartype{t_int, t_uint, t_long, t_ulong, t_float, t_char, t_byte};


void breakpoint()
{
    breakpoint(-1);
}
void breakpoint(int id)
{
    breakpoint(String(id));
}
void breakpoint(String name)
{
    log(String("breakpoint ") +  name);

    // Wait for an answer on serial
    while (true)
    {
        while (DEBUGGER_SERIAL.peek() == -1) ;
        String ret = DEBUGGER_SERIAL.readString();

        // Parse string and look for answer
        ret.toLowerCase();
        if (ret == String("ok")) break;
    }
}


void log(String text)
{
    if (DEBUGGER_SERIAL)
    {
        DEBUGGER_SERIAL.println(String("log ") + text);
    }
}

void log_variable(String name, char variable)
{
    log(String("variable ") + name + String(" ") + String(variable));
}
void log_variable(String name, int variable)
{
    log(String("variable ") + name + String(" ") + String(variable));
}
void log_variable(String name, long variable)
{
    log(String("variable ") + name + String(" ") + String(variable));
}
void log_variable(String name, float variable)
{
    log(String("variable ") + name + String(" ") + String(variable, DEBUGGER_FLOAT_DECIMALS));
}

/**
 * Helper funktion for some data conversion
 */
String to_string(byte* data, DebuggerVartype type)
{
    switch (type)
    {
    case DebuggerVartype::t_int:
        return String((int) *(int*)data);
    case DebuggerVartype::t_uint:
        return String((unsigned int) *(unsigned int*)data);
    case DebuggerVartype::t_long:
        return String((long) *(long*)data);
    case DebuggerVartype::t_ulong:
        return String((unsigned long) *(unsigned long*)data);
    case DebuggerVartype::t_float:
        return String((float) *(float*)data, DEBUGGER_FLOAT_DECIMALS);
    case DebuggerVartype::t_char:
        return String((char) *(char*)data);
    case DebuggerVartype::t_byte:
        return String((byte) *(byte*)data);
    default:
        return String("");
    }
}

void log_array(String name, byte* data, int len, DebuggerVartype type, unsigned int typelen)
{
    String text = String("variable ");
    text += name + String(" [");
    if (len > 0) 
    {
        text += to_string(&data[0], type);
        for (int i=typelen; i<len*typelen; i+=typelen) text += String(",") + to_string(&data[i], type);
    }
    text += String("]");
    log(text);
}
void log_array(String name, int* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_int, sizeof(int));
}
void log_array(String name, unsigned int* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_uint, sizeof(unsigned int));
}
void log_array(String name, long* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_long, sizeof(long));
}
void log_array(String name, unsigned long* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_ulong, sizeof(unsigned long));
}
void log_array(String name, float* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_float, sizeof(float));
}
void log_array(String name, char* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_char, sizeof(char));
}
void log_array(String name, byte* variable, int len)
{
    log_array(name, (byte*)variable, len, DebuggerVartype::t_byte, sizeof(byte));
}


void log_register(String name, char* data, int len)
{
    // allocate maximum needed amount of memory
    byte new_data[sizeof(long)*8];
    // build up new array for nice serial printing as array
    for (int i=0; i<len; i++)
    {
        for (int j=0; j<8; j++)
        {
            new_data[i*8+j] = (byte)((data[i] >> j) & 0x1);
        }
    }
    // use array logging
    log_array(name, new_data, len*8, DebuggerVartype::t_byte, sizeof(byte));
}
void log_register(String name, byte reg)
{
    log_register(name, (char*)&reg, sizeof(byte));
}
void log_register(String name, char reg)
{
    log_register(name, &reg, sizeof(char));
}
void log_register(String name, unsigned int reg)
{
    log_register(name, (char*)&reg, sizeof(unsigned int));    
}
void log_register(String name, unsigned long reg)
{
    log_register(name, (char*)&reg, sizeof(unsigned long));    
}
