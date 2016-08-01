#pragma once

#include "ofMain.h"
#include "IPVideoGrabber.h"
#include "ofxARTPattern.h"
#include "ofxUDPManager.h"
//#include "ofxAssimpModelLoader.h"

using namespace ofx;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    string mjpegStreamURL, udpHost;
    unsigned short udpPort;
    
    ofxArtool5::PatternTracker artk;
    std::shared_ptr<Video::IPVideoGrabber> grabber;
    ofxUDPManager udpConnection;
    void videoResized(const void* sender, ofResizeEventArgs& arg);
		
    class nodeDrone :public ofNode{
        void customDraw(){
            ofPushStyle();
            ofSetColor(255, 0, 0);
            ofFill();
            ofDrawBox(15, 5, 15);
            ofSetColor(0, 0, 0);
            ofNoFill();
            ofDrawBox(15, 5, 15);
            ofPopStyle();
        }
    }drone;
    
    ofLight light;
};
