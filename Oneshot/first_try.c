#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>

MCP_CAN CAN(10);                                                        // pin for CAN could also be 9 or other

void setup()                                                                  // Initialisation routine
{CAN.begin(CAN_125KBPS);}                                          // set canbus speed to 125 kbps

void loop()                                                                   // main program (LOOP)
{
unsigned char stmp1[8] = {0x14, 0x14, 0x71, 0x11, 0x08, 0x20, 0x00, 0x00};    //this is the serial number of the flatpack followed by two 00 bytes 
CAN.sendMsgBuf(0x05004804, 1, 8, stmp1);                                      //send message to log in
unsigned char stmp2[8] = {0xA0, 0x00, 0x80, 0x16, 0x80, 0x16, 0x3E, 0x17};    //set rectifier to 16.0 amps (00 A0) 57.60 (16 80) and OVP to 59.5V (17 3E)
CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp2); 
delay(2000);                                                                  //2 second delay 
}
