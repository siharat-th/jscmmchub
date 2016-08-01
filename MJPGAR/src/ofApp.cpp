#include "ofApp.h"

const float markerDistane = 500.0f;
const float speed = 2.0f;

//--------------------------------------------------------------
void ofApp::setup(){
//    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(60);
    
    
//    ofVec3f euler = drone.getOrientationEuler();
//    float yaw   = euler.x;
//    float pitch = euler.y;
//    float roll  = euler.z;
    
    
    drone.setPosition(0, -15, -50);
    light.setup();
    light.setDirectional();
//    light.enable();
    
    // read config.txt
    ofBuffer buffer = ofBufferFromFile("config.txt");
    if(buffer.size()){
        while(!buffer.isLastLine()){
            string line = buffer.getNextLine();
            if(!line.empty()){
                
                int length = line.length();
                char str[length];
                
                if(line.find("mjpegStreamURL")<length){
                    sscanf(line.c_str(), "mjpegStreamURL = %s", str);
                    mjpegStreamURL = str;
                }
                else if(line.find("udpHost")<length){
                    sscanf(line.c_str(), "udpHost = %s", str);
                    udpHost = str;
                }
                else if(line.find("udpPort")<length){
                    sscanf(line.c_str(), "udpPort = %s", str);
                    udpPort = ofToInt(str);
                }
            }
        }
    }
    
    grabber = std::make_shared<Video::IPVideoGrabber>();
    grabber->setCameraName("LinkitCAM");
    grabber->setURI(mjpegStreamURL);
    grabber->connect();
    
    udpConnection.Create();
    udpConnection.Connect(udpHost.c_str(), udpPort);
    udpConnection.SetNonBlocking(true);
    
    float cameraWidth = 640;//grabber->getWidth();
    float cameraHeight = 480;//grabber->getHeight();
    artk.setup(ofVec2f(cameraWidth, cameraHeight), ofVec2f(ofGetWidth(), ofGetHeight()));
}

void ofApp::videoResized(const void* sender, ofResizeEventArgs& arg)
{
//    if (sender == grabber)
    {
        std::stringstream ss;
        ss << "videoResized: ";
        ss << "Camera connected to: " << grabber->getURI() + " ";
        ss << "New DIM = " << arg.width << "/" << arg.height;
        ofLogVerbose("ofApp") << ss.str();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber->update();
    if(grabber->isFrameNew()){
        artk.update(grabber->getPixels());
    }
    
    if(artk.isFound()){
        // postion from matrix
        ARdouble mat[16], proj[16];
        artk.getARModelMatrix(mat);
        ofVec3f pos(mat[12], mat[13], mat[14]);
        ofVec3f dir(mat[8], mat[9], mat[10]);
        
        ofMatrix4x4 mLookat = ofMatrix4x4::newLookAtMatrix(ofVec3f(0,0,0), pos, ofVec3f(0, 1, 0));
        ofQuaternion qLookat;
        qLookat.set(mLookat);
        
        ofVec3f euler = qLookat.getEuler();
        int yaw = -euler.y;
        int pitch = euler.x;
        int roll = euler.z;
        
        ofQuaternion qNow = drone.getOrientationQuat();
        ofQuaternion qLerp;
        qLerp.slerp(0.1, qNow, qLookat);
        
        drone.setOrientation(qLerp);
        
        ofVec3f desirePosition = pos + dir * markerDistane;
        ofVec3f curPosition = drone.getPosition();
        ofVec3f targetDir = desirePosition - curPosition;
        if(targetDir.length()>=speed){
            targetDir.normalize();
            ofVec3f nextPosition = curPosition + targetDir * speed;
            ofVec3f lerpPosition = curPosition.interpolate(nextPosition, 0.6);
            drone.setPosition(lerpPosition);
        }
        
        char buf[128];
        memset(buf, '\0', 128);
        //        sprintf(buf,
        //                "target_position=%.0f,%.0f,%.0f target_direction=%.3f,%.3f,%.3f euler=%d,%d,%d,",
        //                trans.x, trans.y, trans.z,
        //                dir.x, dir.y, dir.z,
        //                yaw, pitch, roll);
        dir=dir*1000.0f;
        sprintf(buf,
                "aa %.0f %.0f %.0f %.0f %.0f %.0f ",
                pos.x, pos.y, pos.z,
                dir.x, dir.y, dir.z
                );
        
        string message(buf);
        cout << message << endl;
        udpConnection.Send(message.c_str(), message.length());
    }
    else
    {
//        ofLogVerbose("artk not found");
        string msg = "NULL";
        cout << "Marker not found." << endl;
        udpConnection.Send(msg.c_str(), msg.length());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0,0,0);
    ofSetHexColor(0xffffff);
    
    grabber->draw(0, 0);
    
    ofEnableDepthTest();
    if(artk.isFound()){
        ofPushMatrix();
        {
            artk.beginAR();
            ofPushStyle();
            ofNoFill();
//            ofDrawBox(0, 0, 20, 40, 40, 40);
            ofPopStyle();
            ofDrawAxis(20);
            artk.endAR();
        }
        ofPopMatrix();
        
        
        // position from matrix
        ARdouble mat[16], proj[16];
        artk.getARModelMatrix(mat);
        ofVec3f trans(mat[12], mat[13], mat[14]);
        ofVec3f dir(mat[8], mat[9], mat[10]);
        artk.getARProjectionMatrix(proj);
//        stringstream ss;
//        ss << "target_position=" << trans.x << "," << trans.y << "," << trans.z << " target_direction="<< dir.x << "," << dir.y << "," << dir.z;
//        string message = ss.str();
//        udpConnection.Send(message.c_str(), message.length());
        
        // euler angle
        ofVec3f desirePosition = trans + dir * markerDistane;
        ofVec3f curPosition(0, 0, 0);
        ofVec3f targetDir = desirePosition - curPosition;
        ofMatrix4x4 mLookat = ofMatrix4x4::newLookAtMatrix(curPosition, desirePosition, ofVec3f(0, 1, 0));
        ofQuaternion qLookat;
        qLookat.set(mLookat);
        ofVec3f euler = qLookat.getEuler();
        int yaw   = euler.y;
        int pitch = euler.x;
        int roll  = euler.z;
        
//        stringstream ss;
//        ss << "target_position=" << (int)trans.x << "," << (int)trans.y << "," << (int)trans.z;
//        ss << " target_direction="<< dir.x << "," << dir.y << "," << dir.z;
//        ss << " euler=" << yaw << "," << pitch << "," << roll;
//        string message = ss.str();
//        udpConnection.Send(message.c_str(), message.length());
        
//        char buf[128];
//        memset(buf, '\0', 128);
////        sprintf(buf,
////                "target_position=%.0f,%.0f,%.0f target_direction=%.3f,%.3f,%.3f euler=%d,%d,%d,",
////                trans.x, trans.y, trans.z,
////                dir.x, dir.y, dir.z,
////                yaw, pitch, roll);
//        sprintf(buf,
//                "%.0f %.0f %.0f %.1f %.1f %.1f %d %d %d ",
//                trans.x, trans.y, trans.z,
//                dir.x, dir.y, dir.z,
//                yaw, pitch, roll);
//        
//        string message(buf);
//        cout << message << endl;
//        udpConnection.Send(message.c_str(), message.length());
    }
    else
    {
//        string msg = "NULL";
//        cout << "Marker not found." << endl;
//        udpConnection.Send(msg.c_str(), msg.length());
    }
    
    // postion from matrix
    ARdouble mat[16], proj[16];
    artk.getARModelMatrix(mat);
    ofVec3f trans(mat[12], mat[13], mat[14]);
    ofVec3f dir(mat[8], mat[9], mat[10]);
    artk.getARProjectionMatrix(proj);
    
//    ofPushView();
//    {
//        glMatrixMode(GL_PROJECTION);
//#ifdef ARDOUBLE_IS_FLOAT
//        glLoadMatrixf(proj);
//#else
//        glLoadMatrixd(proj);
//#endif
//        glMatrixMode(GL_MODELVIEW);
//        
//        glLoadIdentity();
//        
//        drone.draw();
//    }
//    ofPopView();
    
    
    artk.drawDebug();
    
    ofDisableDepthTest();
    
    
    float kbps = grabber->getBitRate() / 1000.0f; // kilobits / second
    
//    float fps = grabber->getFrameRate();
    float fps = ofGetFrameRate();
    
    std::stringstream ss;
    
    // ofToString formatting available in 0072+
    ss << "NAME: " << grabber->getCameraName() << std::endl;
    ss << "HOST: " << grabber->getHost() << std::endl;
    ss << " FPS: " << ofToString(fps,  2/*,13,' '*/) << std::endl;
    ss << "Kb/S: " << ofToString(kbps, 2/*,13,' '*/) << std::endl;
    
    ofDrawBitmapString(ss.str(), 10, 10+16);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
