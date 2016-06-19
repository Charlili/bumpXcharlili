#include "ncVideoGrabber.h"

//--------------------------------------------------------------
void ncVideoGrabber::setup(int _width, int _height){

    // List devices to get correct deviceIDs
    // https://forum.openframeworks.cc/t/opening-multiple-cameras-using-ofvideograbber/13880/2
    /*
     vector<ofVideoDevice> devices = grabber1.listDevices();
     for(int i=0; i<devices.size(); i++){
     cout << devices[i].id << ": " << devices[i].deviceName;
     if(devices[i].bAvailable) cout << "" << endl;
     else cout << " - UNAVAILABLE " << endl;
     }
     
     grabber1.setDeviceID(0);
     grabber2.setDeviceID(0);
     */
    
    // Initialize grabbers and
    // composite video texture
    
    int w = _width;
    int h = _height;
    
    grabber1.initGrabber(w, h);
    grabber2.initGrabber(w, h);
    texture.allocate(2*w, h, GL_RGB);
    
}

//--------------------------------------------------------------
void ncVideoGrabber::update(){

    // Get camera updates
    // and merge the image
    
    grabber1.update();
    grabber2.update();
    
    // Draw both streams to
    // temp output texture
    
    int w = texture.getWidth() * 0.5;
    int h = texture.getHeight();
    
    texture.begin();
    if(grabber1.isFrameNew()) grabber1.draw(w, 0, -w, h);
    if(grabber2.isFrameNew()) grabber2.draw(2*w, 0, -w, h);
    
    // Temp mirroring the first cam
    grabber1.draw(0, 0, w, h);
    
    texture.end();
    
}

//--------------------------------------------------------------
void ncVideoGrabber::draw(int x, int y){
    
    int w = texture.getWidth() * 0.5;
    int h = texture.getHeight() * 0.5;
    
    texture.draw(x, y, w, h);
    
}

//--------------------------------------------------------------
ofFbo &ncVideoGrabber::getTexture(){
    
    return texture;
    
}