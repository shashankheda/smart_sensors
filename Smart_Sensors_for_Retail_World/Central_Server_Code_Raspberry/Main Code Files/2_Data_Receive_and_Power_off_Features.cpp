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
/*    This Program is a simple program that receives the sensor module data for analysis and switches the modules off when the lights are swiched off */
/*    This program doesn't have a power_on function(). It sends Power_off signal to all the sensor modules so that they get deactivated. When it stops sending Power_off signals (10101010), they get activated.
      PS: After getting deactivated, the sensor modules keep checking after every 60 minutes whether they should still remain off or should they get started
      
/* Including all the required files
   SPI.h is to interface the in-out Pins,
   RF24.h is for interfacing any kind of RF module with a microcontroller
   nRF24L01.h is for specific nRF module related commands
   printf.h is in order that printf() function may be used for printing anything if needed (It is more flexible than Serial.print)
*/
#include <cstdlib>
#include <time.h>
#include <iostream>
#include<wiringPi.h>
#include "../RF24.h"
RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI

//the pipe addresses must be the same as declared on the sensor nodes (We can have 6 pipes at max)
//When data is sent through a pipe address, say pipe number 1 of Raspberry nRF24L01+ module, then it can only be received by a pipe on the sensor nodes which has the same address as pipe number 1 of Raspberry Pi.
const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL };

int count=0;

//Initial setup of GPIO Pins and pipe modes (read/write).
void setup(void)
{
wiringPiSetup();
pinMode(1,INPUT);
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

//This part of program is executed forever. It is assumed that the Raspberry Pi is powered on 24 x 7 so that program runs forever.
unsigned long setbit = 0;
void loop(void)
{
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    radio.openReadingPipe(2,pipes[2]);
    radio.openReadingPipe(3,pipes[3]);

	time_t t;
	FILE *F1;
	F1 = fopen("sensordata.txt","a");

        //If the photoresistor output is low == Surrounding lights are on
	if(digitalRead(1) == 0)
	{
		if ( radio.available() )                                                                                    //If any data packet is available on "ANY PIPE"
		{
			unsigned long got_time;
			bool done = false;
			while (!done)
      			{
		               done = radio.read( &got_time, sizeof(unsigned long) );                                       //Read the data
			        printf("Got payload %lu...",got_time);
			        delay(20);
			      radio.stopListening();

				int model_num = got_time%1000;                                                              //Finding out the node number cause the format in which data is sent is ~ timing_data * 1000 + node_number
				time(&t);
				fprintf(F1,"%d       %d       %lu seconds       %s\n",srnum,model_num,got_time/1000,ctime(&t)); //putting into a file to keep a log. Improve the data logging based on your requirements. timing_data = got_time/1000
				fflush(F1);
				radio.openWritingPipe(pipes[model_num-1]);                                                  //I must send back the confirmation to the sensor_node from which I received the data

/* 
Remember: The pipes on node 1 are same as pipe 0 and 1 of Raspberry Pi nRF module.
          The pipes on node 2 are same as pipe 1 and 2 of Raspberry Pi nRF module.
          The pipes on node 3 are same as pipe 2 and 3 of Raspberry Pi nRF module.
          0xF0F0F0F0E1LL: Node 1 Receiver
          0xF0F0F0F0E2LL: Node 1 transmitter, Node 2 Receiver
          0xF0F0F0F0E3LL: Node 2 transmitter, Node 3 Receiver
          0xF0F0F0F0E4LL: Node 3 transmitter
          The pipes in Raspberry Pi must be activated based on our need.
          Radio.available() function can read data on any pipe, i.e., all pipes become listeners (receivers)
*/

			      printf("Sent response.\n\r");
			      radio.write( &got_time, sizeof(unsigned long) );                                               //Sent the data received confirmation
			      radio.startListening();                                                                        //Start listening for data again
		         }
		}
	}

        //If the photoresistor output is High == Surrounding lights are off
	else if(digitalRead(1) == 1)
	{
  		for(int i=1; i<=4; i++)
		{
                        if(digitalRead(1) == 0) break;                                                                        //break if lights are switched on
			while(1)
			{
				if(digitalRead(1) == 0) break;                                                                //break if lights are switched on
				radio.stopListening();                                                                        //You MUST STOP LISTENING before any operation to be performed on nRF module. Otherwise, error creep in due to unwanted delays
				unsigned long time = 10101010;
//                              printf("Now sending %lu to node %d...",time,i);
                                radio.openWritingPipe(pipes[i-1]);                                                            //Send shudown signal to each pipe

        			bool ok = radio.write( &time, sizeof(unsigned long) );
		            radio.startListening();                                                                           //Start listening for Confirmation for shutdown from the sensor nodes
		            unsigned long started_waiting_at = millis();
		            bool timeout = false;
		            while ( ! radio.available() && ! timeout )
		               if (millis() - started_waiting_at > 200 )
		                  timeout = true;
				unsigned long setbit;
		            if ( timeout )
		            {
//			              printf("Failed, response timed out.\n\r");
            	            }
		            else
		            {
			              radio.read( &setbit, sizeof(unsigned long) );
//			              printf("Got response %lu\n\r",setbit);                                                  //setbit value may be checked for debugging. But if your program is correct, you need not confirm. IDeally, you must do double-check by checking if setbit == node number from which confirmation is received
		            }
                            break;
			}
	          }
	}
	fclose(F1);
}

//Main Program
int main(int argc, char** argv)
{
        setup();          //Setting Up the devices
        while(1)
                loop();   //Loop (runs forever)

        return 0;
}
