Contents
=========
This Folder (Main Code Files) contains:
* Outputs generated Folder which contains a few example outputs generated during testing
* .cpp File to be compiled on Raspberry Pi for intefacing with nRF24L01+ module.
* wiringpi.h file is missing and will have to be installed separately.
* There are 3 files:
	* 1_Data_Receive_Feature.cpp simply receives timing data from nodes and keeps a log in a file
	* 2_Data_Receive_and_Power_off_Features.cpp receives timing data from nodes, keeps a log and deactivates the sensors once the surrounding lights go off. It continuously sends the deactivation signal "10101010" until the lights are on. The Sensor nodes once deactivatd, check back after an hour to see if they have to remain deactivated or get activated.
	* 3_Data_Receive_and_Power_on_and_Power_off_Features.cpp is an improved version of 2_Data_Receive_and_Power_off_Features.cpp. It does not send the deactivation signal continuously. The signal is sent only uptil all the nodes are deactivated. It sends back power on signal (continuously) when surrounding lights are back on.

PS: 
* Please install wiringPi.h in your computer as mentioned by me in 'Software Requirements' on our shopsense.co/wiki page. Link to wiringPi.h installation is:
https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/

* Please note that you may get conflicts between RF24.h and nRF24L01.h (I have also mentioned on our shopsense/wiki page). Read the lines carefully in the error messages shown when you execute the command. It might be displaying two declarations for pinMode, digitalWrite, delay, delayMicroseconds in RF24.h and wiringPi.h.
		To resolve those issues, go to the folder where the wiringi.h is installed (may be /usr/bin folder). Comment out the lines where the above four functions are declared in wiringPi.c and wiringPi.h (may be you could comment them out in nRF24L01.h also [I haven't tried]) and use it to run your program.

* To run the program, use:

	`gcc Easy.cpp -o easy -lrf24 -lwiringPi`
	
	`./easy`


Pin Configuration
=================
In my examples I used /dev/spidev0.0 and GPIO25
I have a model 2 rpi so you should check if the pins are on the same spot

1. nrf-vcc = rpi-3v3 (1)

2. nrf-gnd = rpi-gnd (6)

3. nrf-ce =  rpi-ce0 (22) 

4. nrf-csn = rpi-gpio25 (24)

5. nrf-sck = rpi-sckl (23)

6. nrf-mo = rpi-mosi (19)

7. nrf-mi = rpi-miso (21)


Known Issues
============
* spidev0.0 or spidev0.1 doesn't seem to work
* Problems with pin configuration (Possibly you might have interchanged pin 22 and pin 24 as they differ on Raspberry Model A and Model B.
* USe the pin config shown above if you are using Raspberry Pi Model B.


Suggestions
============
* Please use the Pin Config shown above. (Pins are interchanged on two different models of Raspberry Pi)
* Visit our shopsense.co/wiki Page for details.
* Visit project wiki page for details about functions used.

Contact (you might not need to as its very easy after you execute everything as above)
===============================================================
Shashank Heda 

Email: 
shashankheda@iitgn.ac.in

Alternate Email: 
shashank_heda@yahoo.co.in
