// =============================================================================
//
// Copyright (c) 2009-2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


//------------------------------------------------------------------------------
void ofApp::setup()
{
	ofSetVerticalSync(true);

	useGrabber = false;
	vidGrabber.setVerbose(false);
	vidGrabber.initGrabber(160,120);

	fingerMovie.loadMovie("movies/fingers_160x120x15fps.mov");
	fingerMovie.play();

	// are we connected to the server - if this fails we 
	// will check every few seconds to see if the server exists
	sender.setVerbose(false);
    
	if(sender.setup("127.0.0.1", 8888))
    {
        connectTime = ofGetElapsedTimeMillis();
    }
	
    // some counter text
    counter = 0.0;

}

//------------------------------------------------------------------------------
void ofApp::update()
{
	
    // prepare our video frames
    fingerMovie.update();
    vidGrabber.update();
    
    if(sender.isConnected())
    {
        if(useGrabber)
        {
            if (vidGrabber.isFrameNew())
            {
                sender.sendFrame(vidGrabber.getPixelsRef());
            }
        }
        else
        {
            if(fingerMovie.isFrameNew())
            {
                sender.sendFrame(fingerMovie.getPixelsRef());
            }
        }
    }
    else
    {
        //if we are not connected lets try and reconnect every 5 seconds
        if((ofGetElapsedTimeMillis() - connectTime) > 5000)
        {
            sender.setup("127.0.0.1", 8888);
        }
    }
}

//------------------------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    
    ofSetColor(127);
    ofRect(190,20,ofGetWidth()-160-40,ofGetHeight()-40);
    
	ofSetColor(255,255,0);
    if(useGrabber)
    {
        ofRect(18,158,164,124);
    } else {
        ofRect(18,18,164,124);
    }
    
    ofSetColor(255);
    
	fingerMovie.draw(20,20);
	vidGrabber.draw(20,160);

    // send some text
    std::string txt = "sent from of => " + ofToString(counter++);
    sender.sendText(txt);
    
    std::stringstream ss;
    ss << "This video is being sent over TCP to port 8888 can be" << endl;
    ss << "received in Jitter by using [jit.net.recv @port 8888]." << endl;
    ss << "See ofxJitterNetworkSender_Example.maxpat for example" << endl;
    ss << "usage. This also works in Jitter 4.5, 5, 6, etc." << endl;
    ss << endl;
    ss << "This message is also being sent:" << endl;
    ss << endl;
    ss << "[" << txt << "]" << endl;
    ss << endl;
    ss << "More complex messages including bangs, lists, etc" << endl;
    ss << "are not fully implemented yet." << endl;
    ss << "" << endl;
    ss << endl;
    ss << "SPACEBAR toggles between video sources." << endl;

	ofDrawBitmapString(ss.str(), 200, 60);
	
}


//------------------------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	useGrabber = !useGrabber;
}
