/**
 * SerialDebugger.cpp - Simple serial debugger for Arduino.
 * Created by Luca Baumann, 2020
 */

#include "SerialDebugger.hpp"

enum DebuggerVartype{t_int, t_uint, t_long, t_ulong, t_float, t_char};


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

void log_variable(String name, int variable)
{
    log(String("variable ") + name + String(" ") + String(variable));
}

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
        return String((float) *(float*)data, 5);
    case DebuggerVartype::t_char:
        return String((char) *(char*)data);
    default:
        return String("");
    }
}

void log_variable(String name, byte* data, int len, DebuggerVartype type, int typelen)
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

void log_variable(String name, int* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_int, sizeof(int));
}

void log_variable(String name, unsigned int* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_uint, sizeof(unsigned int));
}

void log_variable(String name, long* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_long, sizeof(long));
}

void log_variable(String name, unsigned long* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_ulong, sizeof(unsigned long));
}

void log_variable(String name, float* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_float, sizeof(float));
}

void log_variable(String name, char* variable, int len)
{
    log_variable(name, (byte*)variable, len, DebuggerVartype::t_char, sizeof(char));
}
