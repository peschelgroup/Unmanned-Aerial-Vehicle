# Manipulator Interface and Control

The files in this project can be used to control a 2 or 3 servo robotic arm using an
iPad app with 2 video inputs.

- iPad: This folder contains a zip file with the contents of an xcode project directory.
	    Everything which was used to make the original iPad app can be found in this zip
		file.
- python: This folder contains the webserver portion of application. This is to be ran on
		  the device which the arduino is attatched to. The webserver runs on port 80 and
		  communicates over usb serial to the arduino.
- arduino: This folder contains the sketch for the arduino to control the 2 or 3 servos
		   which make up the arm. The gripper servo is optional, but the code still exists
		   for it to operate.
- video: This folder contains the c++ code required to compile together a linux executable
		 which will stream video over udp. 

### Usage
1. On the machine which the arduino is connected to, run the server.py file
	- 'sudo python main.py <name>': Whatever the output file should be named
		- The output file contains all the input from the webserver
2. On the machine which the two webcams are connected to, should be the same machine as in
   step 1, run the client file.
	- './client <iPad ipaddress> 5000': port 5000 is the port which the ipad app is listening on for video
		- <iPad ipaddress>: This is the iPads local ipaddress
3. On the iPad, open up the Robotic Arm Interface app. In the top text box is the ipaddress
   "192.168.0.103". Delete this and type in the ipaddress of the machine which is running the
   webserver.
	- "192.168.0.103" was the static ipaddress we used which is why it is the default
	- The ip entered should be the machine's local ip
	- The app will not load unless it is recieving video data over port 5000
