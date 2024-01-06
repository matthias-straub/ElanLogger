# ElanLogger
Lotus Elan M100 ALDL data logger based on Arduino that logs directly to an SD card in ElanScan's .ecu-format.

Join the community at https://www.lotuselancentral.com

This is a data logger based on Arduino that logs directly to an SD card in ElanScan's .ecu-format.
It is small, very simple to build, runs on power supplied by the ALDL connector and can easily stay inside the car hidden below the dashboard in the passenger footwell.
You should be able to build it for reasonable 30 GBP/Euros.

So if you are not afraid of frying your Arduino board or your ECU, please read on.

What you will need (with prices I paid online for reference):

Data logger board:

- 1 Arduino Uno R3 or compatible - €8
- 1 Arduino data logger shield with RTC - €8
- 1 SD card 2GB or less - €5

Interface cable:

- 1 Opel OBD1 to OBD2 adapter, 10 pin - €8
- 2 resistors, 100 ohms - €.2
- 1 zener diode (1N4733A) - €.1
- 1 on-off switch - €1
- 4 Arduino jumper cables - €.5

Software:

- 1 ElanLogger Arduino sketch - free for M100 owners and attached below

Building instructions:

Build the interface cable and connect it to the data logger shield:
1. Cut the OBD1 adapter cable in half and toss the OBD2 part.
2. Identify the 3 required lines A,F and G on the cable.
3. Connect A,F and G to the 4 jumper cables that will connect to the Arduino PINs on the data logger shield "tx","rx","gnd" and "V in" like this:

- A goes straight to "gnd" (this is ground)
- F goes to "V in" with an "on/off"-switch in between (this is the 12v power supply)
- G goes to both "tx" and "rx" via a 100 ohms resistor each (for half-duplex serial data)
- G also goes to "gnd" via a 1N4733A diode (direction "gnd" to G)

Put the ElanLogger software on the Arduino Uno board:
NOTE: to load the software onto the Arduino Uno, its RX and TX pins need to be open and must not be attached to the resistors on the interface cable.
1. install the Arduino IDE on your PC and connect to your Arduino board via USB
2. change the IDE's serial buffer size from 64 to 128 bytes in HardwareSerial.h (see http://www.hobbytronics.co.uk/arduino-s ... uffer-size)
3. install Arduino libraries SDFat (use version 1.0.3) and RTClib
4. open the ElanLogger sketch file (source code) and load it onto your Arduino Uno board

Assemble ElanLogger right after loading the sketch onto the Arduino Uno board:
1. Put the data logger shield (with the interface cable attached) on the Arduino board and insert the fat16 or fat32 formatted sd card.
2. Connect the whole assembly to USB again, so that the internal clock of the data logger shield can initialize with the time that the sketch file has been loaded onto the Arduino Uno

You are done building. Now you are ready to

Use your data logger:
1. Connect your data logger to your Elan's ALDL interface
2. Turn on ignition or start your Elan
3. Turn on your data logger with its on/off-switch
4. Take your Elan for a spin
5. Turn off the ignition
6. Wait for 3 seconds
7. Turn off your data logger
8. Take out the sd card and put it in your PC to open the created .ecu file with ElanScan


Notes:

- It is important to end logging by turning off the ignition of the car first. Only then the data logger will be able to write the necessary header information and close the file.
If you fail to do so, i.e. you turn off the data logger before turning off the ignition, your log file will be incomplete. You will then find a file with extension .ec_ instead of .ecu.

- Do not forget to change the serial buffer size in the Arduino IDE - this is needed to be able to receive all mode 1 data in one go. Without doing so, your .ecu files will be truncated.

- Log file names are formatted YMddhhmm.ecu (Year,Month,day,hour,minute - only last digit for year and month).

- ElanLogger will currently log around 6-7 packets per second. I have not yet tried pushing logging frequency to the edge.

- The internal clock on ElanLogger will initialize with the time that the sketch has been loaded onto the Arduino board. To reset date and time, take the battery out and install the sketch again.

EDIT July 18th 2017:
Replaced ElanLogger.ino with an updated version 1.1
This new version will visually indicate success by flashing the builtin LED in one second intervals after the file has been closed (ignition has been turned off, i.e. the car stopped talking).
Any error (e.g. no sd card inserted) will be indicated by a rapidly flashing LED.

EDIT April 6th 2020:
Re-arranged instructions to reflect that the software needs to be loaded on the Arduino Uno board before assembling the interface cable.

EDIT Sept 30th 2021:
Added version 1.0.3 recommendation for SDFat. Newer versions might cause corrupt .ecu-files ("Invalid floating point operation." error in ElanScan).

EDIT Oct 4th 2021:
Updated ElanLogger.ino with stability improvements.

EDIT Aug 24th 2022:
If you have trouble finding the right spot to change the serial buffer size in your Arduino IDE, here is a hint:
On Windows I simply changed it globally in [YOUR_IDE_INSTALL_DIR]\hardware\arduino\avr\cores\arduino\HardwareSerial.h
Replace the line
#define SERIAL_RX_BUFFER_SIZE 64
with
#define SERIAL_RX_BUFFER_SIZE 128
