#include <cstdlib>
#include <iostream>
#include <time.h>
#include "../RF24.h"
#include "wiringPi.h"

RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI

const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL };

unsigned long lights_off (uint8_t off_num);
unsigned long lights_on (uint8_t on_num);

int  srno = 0;
int trigger_off = 0;
void setup(void)
{
//  FILE *F1;
//  F1 = open("3_sensors_database.csv","a");
  printf("\n\rProgram starts\n\r");
//  fprintf(F1, "

  radio.begin();

  radio.setRetries(15,15);

  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);

//  radio.printDetails();

}

void loop(void)
{
	time_t t;

      FILE *F1;
      FILE *F2;
      F1=fopen("3_sensors_database.txt","a");
      F2=fopen("node_status_database.txt","a");

	int value = digitalRead(1);

	/*if (value == 0 && trigger_off == 1)
	{
		uint8_t on_num = 1;
		unsigned long val_on;
		int count=0;
		while(on_num <=2)
		{
			val_on = lights_on (on_num);
			delay(200);
			if(val_on == on_num)
			{
				radio.stopListening();
				printf("......................Node %i online...............................\n");
				time(&t);
				fprintf(F2, "Node %i online at %s\n",on_num,ctime(&t));
				fflush(F2);
				on_num++;
				count = 0;
				printf("count = %d",count);
			}
			else
			{
				radio.stopListening();
				printf("................................Node %i failed...............count = %d..........on_num = %i..........\n",on_num,count,on_num);
				count++;
				if(count >=5)
				{
					on_num++;
					count = 0;
				}
			}
		}
		trigger_off = 0;
	}*/

	if (value == 0 && trigger_off == 0)
	{
		radio.startListening();
		uint8_t  incoming;
		if(radio.available(&incoming) )
		{
			printf("\nTime to hear from them");
			printf("\nIncoming");
			srno++;
			unsigned long received_data;
			bool done = false;

			while (!done)
			{
				done = radio.read(&received_data, sizeof(unsigned long) );
				delay(20);
			}

			radio.stopListening();
			radio.openWritingPipe(pipes[incoming-1]);
                        time(&t);
			radio.write( &received_data, sizeof(unsigned long) );
			fprintf(F1, "%i,%i,%lu,%s", srno,incoming,(received_data-incoming)/1000,ctime(&t));
			fflush(F1);
			radio.startListening();
		}
		trigger_off = 0;
	}

	else if (value == 1 && trigger_off == 0)
	{
		radio.stopListening();
		uint8_t off_num = 1;
		unsigned long val_off;
		int count = 0;
                while(off_num <=2 && count <=5)
                {
			printf("Closing all\n");
                        val_off = lights_off (off_num);
                        if(val_off == off_num)
                        {
				radio.stopListening();
                                printf("Node %i offline\n");
                                time(&t);
				off_num++;
                                fprintf(F2,"Node %i offine at %s,",off_num, ctime(&t));
				fflush(F2);
				count =0;
                        }
                        else
                        {
				radio.stopListening();
                                printf("Node %i failed",off_num);
				count++;
                        }
                }
                trigger_off = 1;
	}
	fclose(F1);
	fclose(F2);
}

unsigned long lights_on (uint8_t on_num)
{
        unsigned long light_on_acceptance;

        //opening node based on function argument
        radio.openWritingPipe(pipes[on_num-1]);
        radio.openReadingPipe(1,pipes[on_num]);
        radio.stopListening();
        unsigned long start = 1615230518;
        printf(".................................Sending instruction to open node %i...",on_num);
        bool ok = radio.write( &start, sizeof(unsigned long) );

        if (ok)
                printf("ok...\r");
        else
                printf("failed...\r");

        //continue listening for the reply
        radio.startListening();

        unsigned long started_waiting_at = __millis();
        bool timeout = false;

        while ( ! radio.available() && ! timeout )
        {
               __msleep(5); //add a small delay to let radio.available $

               if (__millis() - started_waiting_at > 300 )
                       timeout = true;
        }

        if ( timeout )
        {
//              printf("...Please switch yourself on\n\r");
        }

        else
        {
                // Grab the response, compare, and send to debugging spew
                radio.read( &light_on_acceptance, sizeof(unsigned long) );
		radio.stopListening();
                // Spew it
                printf("...Got response %lu\n\r",light_on_acceptance);
        }
        sleep(1);
	radio.stopListening();
        return light_on_acceptance;
}

unsigned long lights_off (uint8_t off_num)
{
                unsigned long light_off_acceptance;

                //opening node based on function argument
                radio.openWritingPipe(pipes[off_num-1]);
                radio.openReadingPipe(1,pipes[off_num]);
                radio.stopListening();
                unsigned long stop = 19082120;
//                printf("Power off Please...to node %i...",stop,off_num);
                bool ok = radio.write( &stop, sizeof(unsigned long) );

                if (ok)
                        printf("ok...\r");
                else
                        printf("failed...\r");

                //continue listening for the reply
                radio.startListening();

                unsigned long started_waiting_at = __millis();
                bool timeout = false;

                while ( ! radio.available() && ! timeout )
                {
                        __msleep(5); //add a small delay to let radio.available to check payload

                        if (__millis() - started_waiting_at > 200 )
                                timeout = true;
                }

                if ( timeout )
                {
//                      printf("Please switch yourself off...\n\r");
                }

                else
                {
                        // Grab the response, compare, and send to debugging spew
                        radio.read( &light_off_acceptance, sizeof(unsigned long) );
                        // Spew it
//                        printf("...Got response %lu...\n\r",light_off_acceptance);
                }
//    delay(1000);
                sleep(1);
                return light_off_acceptance;
}


int main(int argc, char** argv)
{
        setup();
        while(1)
	{
                loop();
	}

        return 0;
}
