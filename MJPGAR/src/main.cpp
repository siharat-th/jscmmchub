#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    // VGA
//	ofSetupOpenGL( 640, 480,OF_WINDOW);
    
//    // QVGA
    ofSetupOpenGL( 640, 480,OF_WINDOW);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
