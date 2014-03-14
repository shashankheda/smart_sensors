Contents
=========
This Folder (Central_Server_Code_Raspberry) contains:
* Example Folder which contains files from which you may learn nRF24L01+ programming
* Main Code Files which contains code and an executable written by me. The code is based on RF24.h, nRF24L01.h and wiringPi.h header files.
* Various header files for usage in the program you may write.
* wiringpi.h file is missing and will have to be installed separately.

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
