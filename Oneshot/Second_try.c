// second_try.c
//
// This program is intended to be ran on a Arduino Leonardo

// ELCAN FlatPack S 48v/1800W programming in a nutshell
// We have to log in to the ELCAN by sending a packet containing our serial in a handshake.
// After this handshake, we wait a bit and set the rectifier to 32 amps and 57,60V
// In the same packet we set Over Voltage Protection to 59,5V so single ELCAN faults at that voltage.
// In basic the programming is done, if we wait 30 seconds after programming the EEPROM has saved
// the values and the ELCAN will have the parameters as default setting!

// Small charging insight: 
// LiPo an LiIon must be charged constant current up to a maximum voltage, then constant voltage until current tapers to ~= 0.
// So we are setting the charger to max amps (30 ish) until max voltage is reached (57,6v per ELCAN, so 115,2v battery) meaning
// the PSU's will start pushing 30 amps until 115,2v is reached and then tapers the current to ~= 0 keeping the voltage at 115,2v.
// Maximum pack voltage is 116,2v so we are reasonably safe, and current starts tapering at 85% SOC. The ELCAN's are almost
// inactive from 90% charge, leaving the last 10% to the onboard charger which finishes charging the nice way.

// If you feel uncomfortable charging the pack with 30 amps (I was), keep in mind the pack is around 12-13kWh at 116,2v max, meaning the amp-hour
// rating is about 12000/116,2=103AH making a 30 amp charge only 0,3C and a LiPo or LiIon can be safely charged at 1C. Maximum charge power
// the pack can dissipate is 105A so the combined charging of a power tank (2,7kW) and the onboard (1,3kW) and the ELCAN's (3,6kW)
// is at 7,6kW which is about 0,65C and thus absolutely safe.

// USAGE:
// Modify the source code by changing the serial number. The serial number is written on the ELCAN's sticker somewhere and must be fed in
// two-byte hex pairs, followed by two trailing 00 bytes. Modify the current and voltage parameters if desired.
// Program the Leonardo, and keep the computer connected for serial messages.
// Hook up the CAN signals to the Leonardo. Power on the ELCAN and wait a second, then power on the Leonardo keeping an eye on the serial monitor.
// The serial monitor shows the current step, and when done starts a 30 second wait loop. When the loop is finished, unplug the ELCAN and power down
// the Leonardo. Unhook the Leonardo and plug in the ELCAN again. CHECK WITH A VOLT METER IF THE VOLTAGE RAMPS TO THE VOLTAGE SET! Ramp-Up takes about 
// 60 seconds. After the voltage stabilises at the set point and it stays there then mission is accomplished.




#include <mcp_can.h>                                                            // CAN libs
#include <mcp_can_dfs.h>                                                        // CAN libs
#include <SPI.h>                                                                // SPI libs

MCP_CAN CAN(10);                                                                // pin for CAN could also be 9 or other

void setup()                                                                    // Initialisation routine
Serial.begin(9600);                                                             // Initialise the serial monitor
Serial.println("Initialising the CAN bus...");                                  // Explain status on serial
{CAN.begin(CAN_125KBPS);}                                                       // set canbus speed to 125 kbps

void loop()                                                                     // main program (LOOP)
{
    Serial.println("Starting main programming loop...");                        // Explain status on serial
    Serial.println("Logging into rectifier...");                                // Explain status on serial
    unsigned char stmp1[8] = {0x14, 0x14, 0x71, 0x11, 0x08, 0x20, 0x00, 0x00};  // this is the serial number of the flatpack followed by two 00 bytes 
    CAN.sendMsgBuf(0x05004804, 1, 8, stmp1);                                    // send message to log in
    Serial.println("Setting current and voltage...");                           // Explain status on serial
    unsigned char stmp2[8] = {0xFF, 0xFF, 0x80, 0x16, 0x80, 0x16, 0x3E, 0x17};  // set rectifier to max (FF FF), 16.0 amps = (00 10) 57.60 (16 80) and OVP to 59.5V (17 3E)
    CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp2); 
    delay(2000);                                                                // 2 second delay 
    Serial.println("Waiting 30 seconds to give EEPROM time to save...");        // Explain status on serial
    for(int teller=30; teller > 0; teller--){                                   // 30 second serial countdown
        Serial.println(teller);
        delay(1000);
    }
    Serial.println("Done! Power down, disconnect Leonardo and test voltages!"); // We are done.
}
