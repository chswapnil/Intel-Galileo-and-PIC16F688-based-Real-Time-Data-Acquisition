Purpose of the Project
-Engineer a concurrent user space application on Yocto Linux to display sensor values from a microcontroller on a webpage
-Design a firmware application for PIC microcontroller on sensor device to support read and write operations
-Implement POSIX threads to read sensor data and display it on a webpage simulatenously

Components
Intel Galileo Generation II
PICkit3
PIC16F688
LDR
LED 
Resistors (10k, 1k, 220 Ohms) 
Crystal (32 kHz)
Lithium ion battery (3.5 V)
Capacitor (10uF) 
Real Time Clock (DS1307)

Software Tools
-MpLab
-Minicom or Putty
-Gedit

Method to Implememt the Project
-Implement the circuit on breadboard as shown in the schematic diagram.
-Connect the Galileo to the Internet through the ethernet
-Connect PIC microcontroller with LDR sensor and Intel Galileo as shown in diagram
-Load the LAB_4 _Galileo.c on Galileo board through USB.
-Load the LAB_4 _PIC.c on PIC16F688.
-Enter 0 for reset, 1 for Ping, 2 for getdata in linux terminal through PUTTY.
