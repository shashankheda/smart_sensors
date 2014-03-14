/* This program has been written for 'Smart Sensors for Retail World' Project by
   Shashank Heda, Intern, Shopsense Retail Pvt. Ltd.
   
   This program connects the 3 components involved in the sensor module developed during the course of the intern.
   Components:
   1. Raspberry Pi GPIO Pins
   2. nRF24L01+ Transceiver
   3. Photoresistor Nodule (3-pin)
   
   Connnections:
   1. [GND]      nRF pin 1: RPi GND (physical pin 25 or 6)
   2. [Vcc 3V3]  nRF pin 2: RPi 3.3 V Vcc (physical pin 1 or 17)
   3. [CE]       nRF pin 3: Physical Pin no. 22 of Raspberry Pi
   4. [CSN]      nRF pin 4: Physical Pin no. 24 of Raspberry Pi
   5. [SCK]      nRF pin 5: Physical Pin no. 23 of Raspberry Pi
   6. [MOSI]     nRF pin 6: Physical Pin no. 19 of Raspberry Pi
   7. [MISO]     nRF pin 7: Physical Pin no. 21 of Raspberry Pi
   8. [INT]      nRF pin 8: Interrupt pin isn't connected to any pin
*/   

/*    This Program is a simple program that receives the sensor module data for analysis (No power on/ off function)
      Sensors are always on
*/
      
// Including all the required files. PS: Install wiringPi.h based on the instructions in README.md in github
#include <cstdlib>
#include <iostream>
#include <time.h>
#include "../RF24.h"
#include "wiringPi.h"

RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CS$

//the pipe addresses must be the same as declared on the sensor nodes (We can have 6 pipes at max)
//When data is sent through a pipe address, say pipe number 1 of Raspberry nRF24L01+ module, then it can only be received by a pipe on the sensor nodes which has the same address as pipe number 1 of Raspberry Pi.

const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL };

//Initial setup of GPIO Pins and pipe modes (read/write).
void setup(void)
{
  radio.begin();
  radio.setRetries(15,15);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    radio.openReadingPipe(2,pipes[2]);
    radio.openReadingPipe(3,pipes[3]);

  radio.startListening();
  radio.printDetails();
}

int count = 0;

//This part of program is executed forever. It is assumed that the Raspberry Pi is powered on 24 x 7 so that program runs forever.
void loop(void)
{
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
        radio.openReadingPipe(2,pipes[2]);
        radio.openReadingPipe(3,pipes[3]);
	time_t t;
	count++;
	FILE *F1;
	F1 = fopen("database.txt","a");
	unsigned long incoming;

	if(radio.available())                                                                //If any data packet is available on "ANY PIPE"
    	{
	      	unsigned long got_time;
      		bool done = false;

		while (!done)
		{
        		done = radio.read( &got_time, sizeof(unsigned long) );              //Read the data
		        printf("Got payload %lu...",got_time);
			time(&t);
			incoming = got_time%1000;                                           //Finding out the node number because the format in which data is sent is ~ timing_data * 1000 + node_number
			unsigned long value = got_time/1000;                                //Finding out the timing data because the format in which data is sent is ~ timing_data * 1000 + node_number
			fprintf(F1, "%i    %lu    %lu     %s", count,incoming,value,ctime(&t));//putting into a file to keep a log. Improve the data logging based on your requirements
		        delay(20);
		}

      // First, stop listening so we can talk
		radio.stopListening();
                radio.openWritingPipe(pipes[incoming-1]);                                  //I must send back the confirmation to the sensor_node from which I received the data

      // Send the final one back.
		printf("Sent response.\n\r");
		radio.write( &got_time, sizeof(unsigned long) );
	      // Now, resume listening so we catch the next packets.
		radio.startListening();
 }
}

//Main Program
int main(int argc, char** argv)
{
        setup();          //Setting Up the devices
        while(1)
                loop();   //Loop (runs forever)

        return 0;
}