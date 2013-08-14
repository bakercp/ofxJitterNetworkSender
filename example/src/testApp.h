#pragma once

#include "ofMain.h"

#include "ofxJitterNetworkSender.h"

class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    
	bool useGrabber;

    ofVideoPlayer 		fingerMovie;
    ofVideoGrabber 		vidGrabber;
    
    ofxJitterNetworkSender sender;

    float counter;
    int   connectTime;
	
};


