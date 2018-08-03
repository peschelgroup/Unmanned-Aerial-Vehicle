/*
 *   C++ UDP socket client for live image upstreaming
 *   Modified from http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 *   Copyright (C) 2015
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PracticalSocket.h"      // For UDPSocket and SocketException
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()
#include <pthread.h>
using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"


int view = 1;

void *getInput(void *vargs){
	string input;
	while(true){
		cout << "Enter in a view (1: First, 2: Second, 3: both) > ";
		cin >> input;
		cout << "\n";
		view = atoi(input.c_str());
	}
}


int main(int argc, char * argv[]) {
    if ((argc < 3) || (argc > 3)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port>\n";
        exit(1);
    }
	
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, getInput, NULL);

    string servAddress = argv[1]; // First arg: server address
    unsigned short servPort = Socket::resolveService(argv[2], "udp");

    try {
        UDPSocket sock;
        int jpegqual =  ENCODE_QUALITY; // Compression Parameter

        Mat frame1, send1;
	Mat frame2, send2;
        vector < uchar > encoded;
	VideoCapture cap1(0);
	VideoCapture cap2(1);
        namedWindow("send", CV_WINDOW_AUTOSIZE);
        if (!cap1.isOpened()) {
            cerr << "OpenCV Failed to open camera";
            exit(1);
        }

        clock_t last_cycle = clock();
        while (1) {
            cap1 >> frame1;
	    cap2 >> frame2;
            if(frame1.size().width==0)continue;//simple integrity check; skip erroneous data...
            resize(frame1, send1, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
	    resize(frame2, send2, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            vector < int > compression_params;
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegqual);
		
	    if(view == 1){
		    imencode(".jpg", send1, encoded, compression_params);
		    imshow("send", send1);
		    int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

		    int ibuf[1];
		    ibuf[0] = total_pack;
		    sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

		    for (int i = 0; i < total_pack; i++){
		        sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
	  	    }
	    }else if(view == 2){
		    imencode(".jpg", send2, encoded, compression_params);
		    imshow("send", send2);
		    int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

		    int ibuf[1];
		    ibuf[0] = total_pack;
		    sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

		    for (int i = 0; i < total_pack; i++){
		        sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
	  	    }
	    }else if(view == 3){
		    hconcat(send1, send2, send1);
		    imencode(".jpg", send1, encoded, compression_params);
		    imshow("send", send1);
		    int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

		    int ibuf[1];
		    ibuf[0] = total_pack;
		    sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

		    for (int i = 0; i < total_pack; i++){
		        sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
	  	    }
	    }
            waitKey(FRAME_INTERVAL);
        }
        // Destructor closes the socket

    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}
