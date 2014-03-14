/* This program has been written for 'Smart Sensors for Retail World' Project by
   Shashank Heda, Intern, Shopsense Retail Pvt. Ltd.
   
   This program connects the 3 components involved in the sensor module developed during the course of the intern.
   Components:
   1. ATMEGA 328P-PU Microcontroller chip programmed using Arduino
   2. nRF24L01+ Transceiver
   3. Infrared Nodule (3-pin)
   
   Connnections:
   1. [GND]      nRF pin 1: GND
   2. [Vcc 3V3]  nRF pin 2: 3.3 V Vcc
   3. [CE]       nRF pin 3: Pin no. 15 of ATMEGA 328P-PU (Digital Pin 9  of Arduino)
   4. [CSN]      nRF pin 4: Pin no. 16 of ATMEGA 328P-PU (Digital Pin 10 of Arduino)
   5. [SCK]      nRF pin 5: Pin no. 19 of ATMEGA 328P-PU (Digital Pin 13 of Arduino)
   6. [MOSI]     nRF pin 6: Pin no. 17 of ATMEGA 328P-PU (Digital Pin 11 of Arduino)
   7. [MISO]     nRF pin 7: Pin no. 18 of ATMEGA 328P-PU (Digital Pin 12 of Arduino)
   8. [INT]      nRF pin 8: Interrupt pin isn't connected to any pin
   
/* This Program is for Node Number 1 for the Smart Sensor Project*/   
      
/* Including all the required files
   SPI.h is to interface the in-out Pins,
   RF24.h is for interfacing any kind of RF module with a microcontroller
   nRF24L01.h is for specific nRF module related commands
   printf.h is in order that printf() function may be used for printing anything if needed (It is more flexible than Serial.print)
*/
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//define your node number here
#define nodenum 1

//Configuring pin 9 and 10 (as per Digital Pin no.) to act as CE (Chip enable) and CSN (Chip Select Not)
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate (These pipe addresses must be the same on both TX and RX pins)
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL };

// The setup of the current running sketch (one time setup)
void setup(void)
{
  Serial.begin(57600);                      //Serial Monitor bps
  pinMode(2, INPUT);                        //The digital Pin 2 on Arduino (Pin 4 on ATMEGA 328P-PU) is used to take the IR Sensor module output (to process IR Sensor Module value output)
  pinMode(7, OUTPUT);                       //The digital Pin 7 on Arduino (Pin 13 on ATMEGA 328P-PU is used to give VCC to IR Sensor Module (controls IR sensor module power)
  radio.begin();                            //Begin nRF module
  radio.setRetries(15,15);                  //No. of Retries

  radio.openWritingPipe(pipes[nodenum]);          //Pipe from which transmission would be done to the receiver on the other hand.
  radio.openReadingPipe(1,pipes[nodenum-1]);        //Pipe from which the values may be received from other transmitters
        
  radio.startListening();                   //Starts listening for any value, i.e., checks if anything is being recieved at the Reading Pipe 0 with address 0xF0F0F0F0E1LL
}

unsigned long t1 = 0;                       //unsigned long variable to calculate the start time when a person comes in front of the module
int counter=1;                              //counter variable (a logic variable) is one of the dummy variables involved in time calculation process
int trigger_off = 1;                        //Another dummy variable to take care of the Power Saving Feature of the Modules

//CAUTION: The power may be controlled depending on other variables. I used it in a way suitable for my use. It must be changed based on conditions and future products of Shopsense

//Part of the Program which runs continuously (loop)
void loop()
{
    if(radio.available())                  //Initially, it is to be checked if anything is being received at pipe 0
    {
        receive_confirm();                 //To confirm by sending back a confirmation message (bits) to the transmitter who sent the information to this receiver
    }    
    
    if(digitalRead(2) == 1)                //If sensor output from IR Sensor module is high
    {
        if(counter == 1)                   //If counter (dummy logic variable) is 1
             t1 = millis()/1000;           //Mark the start of the timing, store initation time in t1
                     
        counter = 0;                       //counter = 0 (dummy logic variable)
    }
           
    else                                   //If sensor output from IR Sensor module is low
    {
        if(counter == 0)
        {
          unsigned long t2 = millis()/1000;//t2 variable to store 
          if((t2 - t1 > 5))                //If there was a transition in output from IR sensor from HIGH to LOW, and if the time spent >  seconds
               data_sending(t2-t1);        //call data sending function to send data to the receiver on the RPi end
        }
        counter = 1;                       //counter = 1 (dummy logic variable)
    }
}

//Function to send data logged (time in seconds about the person standing in front) to the receiver (attached to Raspberry Pi in my case) and get confirmation about the same
void data_sending(unsigned long timing)
{
    unsigned long response;
    int count = 0;
    while((response != 1000*timing + nodenum) && count<=10)            //Keeps sending data to the receiver (on raspberry end) until it sends a confirmation back or until data is sent 10 times [whichever occurs first]
    {
        count++;                                                       //Counting to see if no. of times data is sent exceeds 10
        radio.stopListening();                                         //stopping the radio to first load data to be sent to the receiver (on Rpi end)
        unsigned long data = 1000*timing + nodenum;                    //Format in which data is sent: timing * 1000 + node number (this node is 1) so that it may be known at receiver end about who sent the data
        bool ok = radio.write( &data, sizeof(unsigned long) );         //Writing the data to send it to the receiver
        radio.startListening();                                        //Start listening for confirmation (which may be sent by RPi end receiver)
        
        if(ok)    printf("Sent");                                      //Double-check to ensure if the data is written or not to the TX pin so that it might be sent from there
        else      printf("failed");
        
        //Waits for 200 ms for getting back the confirmation, otherwise declares timeout and the loop starts again
        unsigned long started_waiting_at = millis();
        bool timeout = false;
        while ( ! radio.available() && ! timeout )
          if (millis() - started_waiting_at > 200 )
            timeout = true;
           
     
        if (timeout ) {}

        else
            radio.read( &response, sizeof(unsigned long) );            //If not timeout, reads the confirmation received from RPi end receiver (that receiver acts as transmitter now)

        delay(500);
    }
        trigger_off = 0;                                               //Sensor module remains activated => trigger_off 0 depicts that state
}

//Function to confirm receival of data whenever a command is received from the transmitter on RPi end
void  receive_confirm()
{
      unsigned long got_data;
      bool done = false;
      while (!done)                                                    //while loop keeps running until some command (data) is received from the Raspberry pi Transmitter
      {
        done = radio.read( &got_data, sizeof(unsigned long) );         //radio.read returns true if a data is available and successfully read which breaks the loop
        delay(20);
      }
      
      if(got_data == 1615230518)                                       //If data received is 1615230518 (Short form for "Power" [nos. are the respective postions of alphabets])
             if(trigger_off == 1)                                      //If the sensor module had been previously deactivated
                   power_on();                                         //Its time to activate (power_on function called)
                          
      else if(got_data == 19082120)                                    //If data received is 19082120 (Short form for "Shut" [nos. are the respective postions of alphabets])
             if(trigger_off == 0)                                      //If the sensor module had been previously activated
                   power_off();                                        //Its time to deactivate (power_off function called)
}

//Power_on function to power on the module
void power_on()
{
  unsigned long got_data = nodenum;                                          //Send confirmation number 1 whenever power on command is received to signify receival of the command
  radio.stopListening();                                               //Stop listening so that data may be loaded for writing out the confirmation to the RPi receiver (was transmitter before)
  radio.write( &got_data, sizeof(unsigned long) );                     //Writing out the confirmation bit 1
  digitalWrite(7,HIGH);                                                //Start providing power to the IR Sensor module
  radio.startListening();                                              //Now, resume listening so we catch the next packets.
  trigger_off = 0;                                                     //Activate the sensor module (dummy variable)
}

//Power_off function to power off the module
void power_off()
{
  unsigned long got_data = nodenum;                                          //Send confirmation number 1 whenever shutdown command is received to signify receival of the command
  radio.stopListening();                                               //Stop listening so that data may be loaded for writing out the confirmation to the RPi receiver (was transmitter before)
  radio.write( &got_data, sizeof(unsigned long) );                     //Writing out the confirmation bit 1
  digitalWrite(7,LOW);                                                 //Stop providing power to the IR Sensor module to save power
  delay(360000);                                                       //Delay of one hour when there would be no activity, minimum power supply from battery
  radio.startListening();                                              //Now, resume listening so we catch the next packets.
  trigger_off = 1;                                                     //Deactivating the sensor module (dummy variable)
}
