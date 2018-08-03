from flask import Flask, render_template, request
from threading import Thread
from subprocess import call, Popen, PIPE
import numpy as np
import cv2
import socket
import random
import serial
import time
import sys
import os

app = Flask(__name__)

#Important for the webpage. Used for data.
templateData = {'title' : 'Robotic Arm Control',
		'gripVal' : '',
		'armPos' : 90,
		'basePos' : 90
		}

#Stores the location of each servo
objectStates = [0, 0, 0]

#Used for comparison. If old != current, send command over serial to change
objectStatesOld = [0, 0, 0]

#Global flag for whether or not the program should try to send a command over serial
send = False

#Global variable for closing threads which aren't the main one. Depreciated.
kill = False

#Global variable which stores the current client which should be interacting with the webpage
clientIP = None

#Global variable used for comparing if the client ip address has changed
preClientIP = None

#Sends a command over serial. Constantly running as a seperate thread. Waits for the states to change
def sendCommand():
	try:
		global send
		p1 = Popen(["ls", "/dev"], stdout=PIPE)
		p2 = Popen(["grep", "ttyA"], stdin=p1.stdout, stdout=PIPE)
		p1.stdout.close()
	
		pFile = ""		
	
		if(not os.path.exists("/home/snuc/participants/")):
			os.mkdir("/home/snuc/participants/")
	
		pFile = open("/home/snuc/participants/" + sys.argv[1] + ".txt", "w+")
			

		ser = serial.Serial(port='/dev/'+str(p2.stdout.read()[0:len(p2.stdout.read())-1]), baudrate=9600, timeout=100)
		ser.isOpen()
		buffer = ''
		out = ''
		while(True):
			if(kill):
				sys.exit(0)
			while(ser.inWaiting() > 0):
				ser.read()

			#x = random.randint(0, 2)
			#objectStates[x] = random.randint(90, 120)
			ser.flush()
			if(True):
				if(objectStates[0] != objectStatesOld[0]):
					#print("Sent")
					ser.write(str('<Base, '+str(objectStates[0])+'>'))
					pFile.write(str('<Base, '+str(objectStates[0])+'>') + "\n")
					pFile.flush()
					os.fsync(pFile.fileno())
					objectStatesOld[0] = objectStates[0]
					print("Base")
				elif(objectStates[1] != objectStatesOld[1]):
					#print("Sent")				
					ser.write(str('<Elbow, '+str(objectStates[1])+'>'))
					pFile.write(str('<Elbow, '+str(objectStates[1])+'>') + "\n")
					pFile.flush()
					os.fsync(pFile.fileno())
					objectStatesOld[1] = objectStates[1]
					print("Elbow")
				elif(objectStates[2] != objectStatesOld[2]):
					if(objectStates[2] == 'checked'):
						val = 160
					else:
						val = 115
					#print("Sent")				
					ser.write(str('<Grip, '+str(val)+'>'))
					pFile.write(str('<Grip, '+str(val)+'>') + "\n")
					pFile.flush()
					os.fsync(pFile.fileno())
					objectStatesOld[2] = objectStates[2]
					print("Grip")									
				send = False
	except Exception:
		print("Arduino cannot be found")
		sys.exit(0)

#Starts the new thread which sends commands over serial to the arduino
try:
	t1 = Thread(target=sendCommand, args=())
	t1.start()
except Exception:
	print("Unable to start thread")
		

@app.route("/", methods=['GET'])
def main():
	global clientIP
	clientIP = request.remote_addr
	return render_template('interface.html', **templateData)

@app.route("/", methods=['POST'])
def action():
	global send
	objectStatesOld[2] = objectStates[2]
	objectStatesOld[1] = objectStates[1]
	objectStatesOld[0] = objectStates[0]

	objectStates[2] = 'checked' if request.form['gripper']=='true' else ''
	objectStates[1] = request.form['armVal']
	objectStates[0] = abs(int(request.form['baseVal'])-180)
	
	templateData['gripVal'] = objectStates[2]
	templateData['armPos'] = objectStates[1]
	templateData['basePos'] = objectStates[0]
	
	send = True
	
	return render_template('interface.html', **templateData)

if __name__ == "__main__":
	try:	
		if(len(sys.argv) == 1):
			print "Participant name not specified"
			sys.exit(0)
		app.run(host='0.0.0.0', port=80, debug=True)
	except Exception:
		print("Port 80 already in use")
		kill = True
		sys.exit(0)


















