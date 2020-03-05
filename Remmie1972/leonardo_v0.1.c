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
#include <LiquidCrystal.h>

word maxcurrent = 100;                                                             // set initial maxcurrent (divide by 10)
word outputvoltage = 5750;                                                         // set output voltage to 57.50 Volt (divide by 100)
word overvoltage = 5950;                                                           // set the overvoltage protection limit at 59.50 Volt (divide by 100)
bool longwalkin = HIGH;                                                            // HIGH is for 60 second walk-in, LOW is for 5 second walkin (watch the capitols !)
word currentstep = 100;

/* THIS IS THE MAIN PROGRAM FOR THE LEONARDO CANBUS BOARD
 *  DO NOT ALTER ANY CODE BELOW UNLESS YOU'RE A PROGRAMMER
 */

const int SPI_CS_PIN = 17;                                                          // set the input pin for the CANBUS receiver (on the leonardoboard = 17)
unsigned char len = 0;                                                              // this variable holds the length of the CANBUS message received
unsigned char serialnr[8];                                                          // this variable holds the serialnumber of the Flatpack
int msgreceived;                                                                    // this variable holds the number of messages received
int led = 23;                                                                       // LED output pin for the leonardo CAN board= 23
int buttondown = 10;
int buttonup = 11;

MCP_CAN CAN(SPI_CS_PIN);                                                            // Set CS pin for CANBUS shield
//LiquidCrystal lcd(1, 3, 4, 5, 6, 7, 8);                                              // LiquidCrystal(rs, rw, enable, d4, d5, d6, d7)  or LiquidCrystal(rs, enable, d4, d5, d6, d7) 

void setup()                                                                         // Initialisation routine
{
 pinMode(buttondown, INPUT);                                                                 // Set pin 10 to input (pushbutton for down)
 digitalWrite(buttondown, HIGH);                                                             // activate pull-up on A0
 pinMode(buttonup, INPUT);                                                                 // Set pin 11 to input (pushbutton for up)
 digitalWrite(buttonup, HIGH);                                                             // activate pull-up on A0
 pinMode(led, OUTPUT);                                                               // pin 23 is the output pin for the LEONARDO CANBUS board
 digitalWrite(led, LOW);                                                             // turn the LED of
/*  lcd.begin(16, 2);                                                                // splash screen on LCD
  lcd.print("Flatpack Charger");
  lcd.setCursor(0, 1);
  lcd.print("Made by RHO");
*/  
 Serial.begin(115200);                                                               // Set the baudrate for the seial connection to the PC at 115200
 
 START_INIT:

    if(CAN_OK == CAN.begin(CAN_125KBPS))                                              // init can bus : baudrate = 125k !!
    {
//        lcd.setCursor(0,0);
//        lcd.print("CAN BUS init OK!");

        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
//        lcd.setCursor(0,0);
//        lcd.print("CAN BUS init Fail");
//        lcd.setCursor(0,1);
//        lcd.print("Init CAN BUS again");
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        goto START_INIT;
    }
//    lcd.clear();
}

void loop()                                                                           // main program (LOOP)
  {
    
  unsigned char buf[8] ;
  if(CAN_MSGAVAIL == CAN.checkReceive())                                              // check if data coming
    {
        digitalWrite(led,HIGH);                                                       // turn the LED on
        CAN.readMsgBuf(&len, buf);                                                    // read data,  len: data length, buf: data buf
        INT32U canId = CAN.getCanId();                                                // read the CAN Id
         Serial.println();                                                            // start on a new line
         Serial.print("Received : 0");                                                // leading zero 
         Serial.print(canId,HEX);                                                     // output CAN Id to serial monitor
         Serial.print("\t");                                                          // send Tab
        for(int i = 0; i<len; i++)                                                    // print the data
        {
            if( buf[i] < 0x10){ Serial.print("0");} Serial.print(buf[i],HEX);         // send a leading zero if only one digit
            Serial.print(" ");                                                        // space to seperate bytes
        }
        digitalWrite(led,LOW);                                                        // turn the LED off


    if(canId==0x05014400)                                                             //this is the request from the Flatpack rectifier during walk-in (start-up) or normal operation when no log-in response has been received for a while
       {
         for(int i = 0; i<8; i++)                                                 
         {
           serialnr[i]=buf[i];                                                          // transfer the message buffer to the serial number variable
         } 
          Serial.print("\tSerial Number is : ");
          for(int i = 0; i<6; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
          }
         digitalWrite(led,HIGH);
         CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                  //send message to log in (DO NOT ASSIGN AN ID use 00)
         digitalWrite(led,LOW);
         Serial.println();
         Serial.print("TRANSMIT : 05004804");
         Serial.print("\t");
         for(int i = 0; i<len; i++)                                                 // print the data
           {
             if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
             Serial.print(" ");                                                     // space to seperate bytes
           }
       Serial.print("\tLog in with SrNr : ");
        for(int i = 0; i<6; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
          }
         
         
         msgreceived++;                                                               // increase the variable "msgreceived" 
         unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), lowByte(outputvoltage), highByte(outputvoltage), lowByte(outputvoltage), highByte(outputvoltage), lowByte(overvoltage), highByte(overvoltage)};    // set rectifier to maxcurrent 57,0V (16 44) and OVP to 59.5V (17 3E) qnd long walk-in 4005 or short walk in 4004
          if(longwalkin == HIGH)
           {        
             digitalWrite(led,HIGH);
             CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
             Serial.println();
             Serial.print("TRANSMIT : 05FF4005");
            }
          else
            {
             digitalWrite(led,HIGH);
              CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
              Serial.println();
              Serial.print("TRANSMIT : 05FF4004");
            }
        Serial.print("\t");
        for(int i = 0; i<len; i++)                                                 // print the data
          {
            if( stmp7[i] < 0x10){ Serial.print("0");} Serial.print(stmp7[i],HEX);      // send a leading zero if only one digit
            Serial.print(" ");                                                     // space to seperate bytes
          }
//        Serial.println();
//        Serial.print("\t");
//        for(int i = 0; i<len; i++)                                                 // print the data
//          {
//            if( stmp7[i] < 0x10){ Serial.print("0");} Serial.print(stmp7[i],HEX);      // send a leading zero if only one digit
//            Serial.print(" ");                                                     // space to seperate bytes
 //         }
        Serial.print("\tImax set to : ");
        Serial.print(0.1*(stmp7[0]+stmp7[1]*256));
        Serial.print(" A\t");
        Serial.print("Vout set to : ");
        Serial.print(0.01*(stmp7[2]+stmp7[3]*256));
        Serial.print(" V\t");
        Serial.print("OVP set to : ");
        Serial.print(0.01*(stmp7[6]+stmp7[7]*256));
        Serial.print(" V\tWalkin : ");
        if(longwalkin == HIGH)
          {        
             Serial.print("60 seconds");
          }
       else
          {
             Serial.print("5 seconds");
          }          
       }

    
    else if(canId==(0x05000000+256*buf[5]+buf[6]))                                    //if CANID = 0500xxyy where xxyy the last 2 digits of the serial nr
      {
        for(int i = 0;i<6; i++)
        {
        serialnr[i] = buf[i+1];  
        }
//       unsigned char serialnr[8] = {buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], 0x00, 0x00};                     //this is the serial number of the unit which is covered in the request (unit announces its serial number)
        Serial.print("\tSerial Number is : ");
        for(int i = 0; i<6; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
          }
        msgreceived++;
              digitalWrite(led,HIGH);
       CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                                      //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
        msgreceived=0;
        Serial.println();
        Serial.print("TRANSMIT : 05004804");
        Serial.print("\t");
       for(int i = 0; i<8; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
            Serial.print(" ");                                                     // space to seperate bytes
          }
        Serial.print("\tLog in with SrNr : ");
        for(int i = 0; i<6; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
          }
             
        unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), lowByte(outputvoltage), highByte(outputvoltage), lowByte(outputvoltage), highByte(outputvoltage), lowByte(overvoltage), highByte(overvoltage)};     // set rectifiers maxcurrent, outputvoltage and OVP and long walk-in 4005 or short walk in 4004
         if(longwalkin == HIGH)
           {        
             digitalWrite(led,HIGH);
             CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
             Serial.println();
             Serial.print("TRANSMIT : 05FF4005");
            }
          else
            {
              digitalWrite(led,HIGH);
             CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
              Serial.println();
              Serial.print("TRANSMIT : 05FF4004");
            }
        Serial.print("\t");
        for(int i = 0; i<len; i++)                                                 // print the data
          {
            if( stmp7[i] < 0x10){ Serial.print("0");} Serial.print(stmp7[i],HEX);      // send a leading zero if only one digit
            Serial.print(" ");                                                     // space to seperate bytes
          }
//        Serial.println();
//        Serial.print("\t");
//        for(int i = 0; i<len; i++)                                                 // print the data
//          {
//            if( stmp7[i] < 0x10){ Serial.print("0");} Serial.print(stmp7[i],HEX);      // send a leading zero if only one digit
//            Serial.print(" ");                                                     // space to seperate bytes
 //         }
        Serial.print("\tImax set to : ");
        Serial.print(0.1*(stmp7[0]+stmp7[1]*256));
        Serial.print(" A\t");
        Serial.print("Vout set to : ");
        Serial.print(0.01*(stmp7[2]+stmp7[3]*256));
        Serial.print(" V\t");
        Serial.print("OVP set to : ");
        Serial.print(0.01*(stmp7[6]+stmp7[7]*256));
        Serial.print(" V\tWalkin : ");
        if(longwalkin == HIGH)
          {        
             Serial.print("60 seconds");
          }
       else
          {
             Serial.print("5 seconds");
          }
       }


    else if((canId==0x05014004)or(canId==0x05014008)or(canId==0x05014010)or(canId=0x0501400C))                        // these are the status messages (05014004 is not current-limiting, 05014008 is current limiting 05014010 = busy with walkin, 0501400C in input voltage low)
      {  
           Serial.print("\t");
           Serial.print("Tin = ");
           Serial.print(buf[0]);                                                        //first byte is temperature in (celcius)
           Serial.print(" C\t");

           Serial.print("Tout = ");
           Serial.print(buf[7]);                                                        //last byte is temperature out (celcius)
           Serial.print(" C\t");

           Serial.print("Vin = ");          
           Serial.print(buf[5]);                                                        // sixth byte is input voltage in volts ac/dc
           Serial.print(" V\t");
   
           Serial.print("Iout = ");
           Serial.print(buf[2]*256*0.1+buf[1]*0.1);                                     // third (msb) and second byte are current in 0.1A (deciamp) calculated to show directly in Amps
           Serial.print(" A\t");
       
           Serial.print("Vout = ");
           Serial.print(buf[4]*256*0.01+buf[3]*0.01);                                   // fifth (msb) and fourth byte are voltage in 0.01V (centivolt) calculated to show directly in Volts dc
           Serial.print(" V\t");
      
           Serial.print("Pout = ");
           Serial.print((2*buf[4]*256*0.01+2*buf[3]*0.01)*(buf[2]*256*0.1+buf[1]*0.1));     // Power is calculated from output voltage and current. Output is in Watts
           Serial.print(" W\t");

           msgreceived++;                                                                                             // record number of messages received
           if(msgreceived>40)                                                                                     
            {
             msgreceived=0;
               digitalWrite(led,HIGH);
           CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                              //send message to log in every 40 messages (DO NOT USE ID NR, USE 00) this because during walk-in the 0500xxyy is not send and the rectifier "logs out" because of no received log-in messages from controller
             digitalWrite(led,LOW);
             Serial.println();
             Serial.print("TRANSMIT : 05004804");
             Serial.print("\t");
             for(int i = 0; i<8; i++)                                                 // print the data
               {
                 if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
                 Serial.print(" ");                                                     // space to seperate bytes
               }
             Serial.print("\tover 40 messages : Log in with SrNr : ");
             for(int i = 0; i<6; i++)                                                 // print the data
               {
                 if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
               }
            msgreceived++;
            }
 
/*           lcd.setCursor(0, 0);                                                        //show data on the LCD screen 0x050140yy 0xAA 0xBB 0xCC 0xDD 0xEE 0xFF 0xGG 0xHH
           lcd.print("T=");
           lcd.print(buf[0]);                                                          // 0xAA = Temperature in
           lcd.print("/");  
           lcd.print(buf[7]);                                                          // 0xHH = Temperature out
           lcd.print("   ");
           lcd.setCursor(8,0);
           lcd.print("I=");
           lcd.print(buf[2]*256*0.1+buf[1]*0.1);                                       // 0xBB = Current Low Byte, 0xCC = Current High byte. Current in deciAmps (*0.1 Amps)
           lcd.print("  ");
              
           lcd.setCursor(8,1);
           lcd.print("Vo=");
           lcd.print(buf[4]*256*0.01+buf[3]*0.01);                                     // 0xDD = Voltage out Low Byte, oxEE = Voltage out High Byte. Voltgae in centivolts (*0.01 Volt)
           lcd.print("  ");
                      
           lcd.setCursor(0,1);
           lcd.print("Vi=");          
           lcd.print(256*buf[6]+buf[5]);                                               // 0xFF = Voltage in Low byte, 0xGG = Voltage in High byte. Voltage in Volts (because voltage is below 255 Volts, high byte is always 0)
           lcd.print("  ");
*/           
           if((digitalRead(buttonup)==0))                                                    //read digital pin for button up and if it is low (pushbutton is pressed) 
             {
             maxcurrent = maxcurrent + currentstep;                                            // raise maxcurrent with 10 A
             if(maxcurrent > 500)                                                      // to be able to lower the current with one button, if the maxcurrent > 50 the current is reset to 10
               {
               maxcurrent =100;
               }
               digitalWrite(led,HIGH);
           CAN.sendMsgBuf(0x05004804, 1, 8, serialnr);                                                                  // send message to log in (DO NOT ASSIGN AN ID) use 00
             digitalWrite(led,LOW);
             Serial.println();
        Serial.print("TRANSMIT : 05004804");
        Serial.print("\t");
       for(int i = 0; i<8; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
            Serial.print(" ");                                                     // space to seperate bytes
          }
        Serial.print("\tLog in with SrNr : ");
        for(int i = 0; i<6; i++)                                                 // print the data
          {
            if( serialnr[i] < 0x10){ Serial.print("0");} Serial.print(serialnr[i],HEX);      // send a leading zero if only one digit
          }
              unsigned char stmp7[8] = {lowByte(maxcurrent), highByte(maxcurrent), lowByte(outputvoltage), highByte(outputvoltage), lowByte(outputvoltage), highByte(outputvoltage), lowByte(overvoltage), highByte(overvoltage)};     // set rectifiers maxcurrent, outputvoltage and OVP and long walk-in 4005 or short walk in 4004
             if(longwalkin == HIGH)
               {        
              digitalWrite(led,HIGH);
                CAN.sendMsgBuf(0x05FF4005, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
                 Serial.println();
                 Serial.print("TRANSMIT : 05FF4005");
               }
             else
               {
                 digitalWrite(led,HIGH);
             CAN.sendMsgBuf(0x05FF4004, 1, 8, stmp7);                                                                     //(last 4 in header is for 5 sec walkin, 5 is for 60 second walkin)
             digitalWrite(led,LOW);
                 Serial.println();
                 Serial.print("TRANSMIT : 05FF4004");
                }
        Serial.print("\t");
        for(int i = 0; i<len; i++)                                                 // print the data
          {
            if( stmp7[i] < 0x10){ Serial.print("0");} Serial.print(stmp7[i],HEX);      // send a leading zero if only one digit
            Serial.print(" ");                                                     // space to seperate bytes
          }
        Serial.print("\tButton pressed : Imax set to : ");
        Serial.print(0.1*(stmp7[0]+stmp7[1]*256));
        Serial.print(" A\t");
        Serial.print("Vout set to : ");
        Serial.print(0.01*(stmp7[2]+stmp7[3]*256));
        Serial.print(" V\t");
        Serial.print("OVP set to : ");
        Serial.print(0.01*(stmp7[6]+stmp7[7]*256));
        Serial.print(" V\tWalkin : ");
        if(longwalkin == HIGH)
          {        
             Serial.print("60 seconds");
          }
       else
          {
             Serial.print("5 seconds");
          }
/*           lcd.setCursor(0, 0);                                                                                         // announce chosen current on LCD
             lcd.print("Current set to  ");
             lcd.setCursor(0, 1);
             lcd.print(maxcurrent*0.1);
             lcd.print(" Amp        ");
*/             delay(1000);
             }       
      } 
    else
      {
//        lcd.setCursor(0,0);                                                            // if the canId is unknown
//        lcd.print(canId,HEX);                                                          // show the can Id and the message on the LCD
//        lcd.setCursor(0,1);
//        for(int i = 0; i<len; i++)                                                    
//          {
//          if( buf[i] < 0x10){ lcd.print("0");} lcd.print(buf[i],HEX);                  // send a leading zero if only one digit
//          }
        Serial.println("\tUNKNOWN COMMAND");
//        delay(1000);                                                                   // show the message for 1 second and then continue
      }
    }
    
  }

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
