# Arduino Serial Debugger
Small library for Arduino debugging over serial port. Features:

- breakpoints: halt execution until "ok" is received over serial.
- logging: logging of simple strings, variables and arrays

SerialDebugger.ino contains sample usage.

gui/SerialDebuggerInterface.py contains a QT GUI to visualize logged variables and skip over breakpoints. 