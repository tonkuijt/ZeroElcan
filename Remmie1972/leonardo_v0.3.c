// Leonardo CAN bus program to program ELCAN rectifiers and read them as well
// Stolen from Remmie1972, modified for serial comms for bench testing by
// Ton Kuijt (https://github.com/tonkuijt/)


// from rectifier : (requests for logins)
// 05014400 + ser nr + 00 00 from rectifier  : HELLOOW where are you ! rectifier sends 05014400 and 6 bytes serial number and followed by 00 00 (login request)
// 0500xxyy + 1B + ser nr + 00 is send during normal voltage every second. xxyy is the last 2 bytes from the serial number
// after either of these send 05004804 every 5 seconds ! to keep logged in. rectifier does not send login requests so after 10 second the numbers stop until 05014400 is sent
// from rectifier : (status messages)
// 0501400C + status data : walkin and below 43 volts (error) and during walk-out (input voltage low)
// 05014010 + status data : walkin busy
// 05014004 + status data : normal voltage reached
// 05014008 + status data : current-limiting active
// send TO rectifier (act as controller)
// 05004804 + ser nr + 00 00 from controller : send 05004804 and 6 bytes ser number followed by 00 00
// 05FF4004 controller sends current and voltage limits (last 4 is 5 sec walk-in, for 60 second walk-in use 05FF4005)
// 05FF4005 controller sends current and voltage limits (last 5 is 60 sec walk-in, for 5 second walk-in use 05FF4004)

#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>

const int SPI_CS_PIN = 10;
word maxcurrent = 0x64;                                                              // set initial maxcurrent to 10A output
unsigned char len = 0;
unsigned char serialnr[8];
int msgreceived;
MCP_CAN CAN(SPI_CS_PIN);                                                             // Set CS pin for CANBUS shield

void setup()                                                                         // Initialisation routine
{
  Serial.begin(115200)                                                                // Serial monitor for lack of LCD
  pinMode(A0, INPUT);                                                                 // Set pin A0 to input (pushbutton)
  digitalWrite(A0, HIGH);                                                             // activate pull-up on A0
  Serial.println("Flatpack Charger");
  Serial.println("Made by RHO, modified by TonKuijt");
   
START_INIT:

    if(CAN_OK == CAN.begin(CAN_125KBPS))                                              // init can bus : baudrate = 125k !!
    {
        Serial.println("CAN BUS init OK!");
        delay(100);
    }
    else
    {
        Serial.println("CAN BUS init Fail");
        Serial.println("Init CAN BUS again");
        delay(100);
        goto START_INIT;
    }
}

void loop()                                                                           // main program (LOOP)
  {
   
  unsigned char buf[8] ;
  if(CAN_MSGAVAIL == CAN.checkReceive())                                              // check if data coming
    {
        CAN.readMsgBuf(&len, buf);                                                    // read data,  len: data length, buf: data buf
        INT32U canId = CAN.getCanId();                                                // read the CAN Id


    if(canId==0x05014400)                                                             //this is the request from the Flatpack rectifier during walk-in (start-up) or normal operation when no log-in response has been received for a while
       {
         for(int i = 0; i<8; i++)                                                 
         {
         serialnr=buf;                                                          // transfer the message buffer to the serial number variable
         }
         CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                  //send message to log in (DO NOT ASSIGN AN ID use 00)
         msgreceived++;                                                               // increase the variable "msgreceived"
         unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), 0x44, 0x16, 0x44, 0x16, 0x3E, 0x17};    // set rectifier to maxcurrent 57,0V (16 44) and OVP to 59.5V (17 3E) qnd long walk-in 4005 or short walk in 4004
         CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
       }

   
    else if(canId==(0x05000000+256*buf[5]+buf[6]))                                    //if CANID = 0500xxyy where xxyy the last 2 digits of the serial nr
      {
        for(int i = 0; i<6; i++)                                                 
          {
            serialnr=buf[i+1];                                                     // transfer the buffer to the serial number variable (neccesary when switching the CAN-bus to another rectifier while on)
          }
        unsigned char serialnr[8] = {buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], 0x00, 0x00};                     //this is the serial number of the unit which is covered in the request (unit announces its serial number)
        CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                                   //send message to log in (DO NOT ASSIGN AN ID) use 00
        msgreceived++;
        unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), 0x44, 0x16, 0x44, 0x16, 0x3E, 0x17};     // set rectifier to maxcurrent 57,0V (16 44) and OVP to 59.5V (17 3E) qnd long walk-in 4005 or short walk in 4004
        CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                      //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
      }


    else if((canId==0x05014004)or(canId==0x05014008)or(canId==0x05014010)or(canId=0x0501400C))                        // these are the status messages (05014004 is not current-limiting, 05014008 is current limiting 05014010 = busy with walkin, 0501400C in input voltage low)
      { 
           msgreceived++;                                                                                             // record number of messages received
           if(msgreceived>40)                                                                                     
            {
             msgreceived=0;
             CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                              //send message to log in every 40 messages (DO NOT USE ID NR, USE 00) this because during walk-in the 0500xxyy is not send and the rectifier "logs out" because of no received log-in messages from controller
             msgreceived++;
            }
           
           Serial.print("T=");
           Serial.print(buf[0]);                                                          // 0xAA = Temperature in
           Serial.print("/");
           Serial.println(buf[7]);                                                          // 0xHH = Temperature out
           Serial.print("I=");
           Serial.print(buf[2]*256*0.1+buf[1]*0.1);                                       // 0xBB = Current Low Byte, 0xCC = Current High byte. Current in deciAmps (*0.1 Amps)             
           
           Serial.print("  Vo=");
           Serial.print(buf[4]*256*0.01+buf[3]*0.01);                                     // 0xDD = Voltage out Low Byte, oxEE = Voltage out High Byte. Voltgae in centivolts (*0.01 Volt)
           Serial.print("  ");
                     
           Serial.print("Vi=");         
           Serial.println(256*buf[6]+buf[5]);                                               // 0xFF = Voltage in Low byte, 0xGG = Voltage in High byte. Voltage in Volts (because voltage is below 255 Volts, high byte is always 0)
           
           if((digitalRead(A0)==0))                                                    //read digital pin Analog0 and if it is high (pushbutton is pressed)
             {
             maxcurrent = maxcurrent + 100;                                            // raise maxcurrent with 10 A
             if(maxcurrent > 400)                                                      // to be able to lower the current with one button, if the maxcurrent > 40 the current is reset to 10
               {
               maxcurrent =100;
               }
             digitalWrite(9,HIGH);                                                                                        // switch on the backlight
             CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                                  // send message to log in (DO NOT ASSIGN AN ID) use 00
             unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), 0x44, 0x16, 0x44, 0x16, 0x3E, 0x17};    // set rectifier to maxcurrent 57,0V (16 44) and OVP to 59.5V (17 3E) qnd long walk-in 4005 or short walk in 4004
             CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                     // (last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             Serial.print("Current set to  ");
             Serial.print(maxcurrent*0.1);
             Serial.println(" Amp");
             delay(1000);
             }       
      }
    else
      {
        Serial.print(canId,HEX);                                                          // show the can Id and the message on the serial monitor
        for(int i = 0; i<len; i++)                                                   
          {
          if( buf < 0x10){ Serial.print("0");} Serial.println(buf,HEX);                  // send a leading zero if only one digit
          }
        delay(1000);                                                                   // show the message for 1 second and then continue
      }
    }
   
  }

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/