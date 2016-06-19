#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(0);
    ncSetup("BUMP-MYSKETCH", 3600, 1080);
    threshold = 30;
    ncRotate = false;
    
    
    // Setup...
    // App stuff
    
    webcam.setup(320, 240);
    rgb.allocate(webcam.grabber1.getWidth(), webcam.grabber1.getHeight());
    grayscale.allocate(webcam.grabber1.getWidth(), webcam.grabber1.getHeight());
    background.allocate(webcam.grabber1.getWidth(), webcam.grabber1.getHeight());
    difference.allocate(webcam.grabber1.getWidth(), webcam.grabber1.getHeight());
    
    
    
    // GUI
    //parameters.add(myVariable.set("Some boolean", true));
    //parameters.add(myClass.parameters);
    gui.setup(parameters);
    
    
}

//--------------------------------------------------------------
void ofApp::update(){

    if(ncPaused) return;
    ofSetWindowTitle("FPS: "+ ofToString(ofGetFrameRate()));
    
    
    
    
    // Update...
    // App stuff
    
    webcam.update();
    getBlobs();
    
    
    // Simple 'camera'
    // Just rotates stuff
    
    if(ncRotate){
        float dX = (float)ofGetMouseX()/(float)ofGetWidth() * 360;
        float dY = (float)ofGetMouseY()/(float)ofGetHeight() * 360;
        ncCamera.x += (dX - ncCamera.x) * 0.05;
        ncCamera.y += (dY - ncCamera.y) * 0.05;
    }
 
}
void ofApp::getBlobs(){
    if(webcam.grabber1.isFrameNew()){
        
        rgb.setFromPixels(webcam.grabber1.getPixels(), webcam.grabber1.getWidth(), webcam.grabber1.getHeight());
        grayscale.mirror(false, true);

        grayscale = rgb;
        difference.absDiff(background, grayscale);
        difference.threshold(threshold);
        
        contour.findContours(difference, 10, 1000, 10, false,true);
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ncScene.begin();
    ofClear(0);
    ofPushMatrix();
    ofEnableDepthTest();
    ofEnableAlphaBlending();

    
    
    
    // Get center point of the screen
    // Can be handy to position stuff
    
    int x = ncScene.getWidth() * 0.5;
    int y = ncScene.getHeight() * 0.5;
    
    ofTranslate(x, y);
    ofRotateY(ncCamera.x);
    ofRotateZ(ncCamera.y);
    
    // Draw...
    // App stuff
    
    ofNoFill();
    ofSetLineWidth(10);
    ofDrawBox(75);
    ofDrawBox(150);
    ofDrawBox(300);
    ofDrawBox(600);
    ofDrawBox(1200);
    
    
    
    
    // Close drawing to FBO, export texture
    // to Syphon and preview our scene
    
    ofDisableAlphaBlending();
    ofDisableDepthTest();
    ofPopMatrix();
    ncScene.end();
    ncServer.publishTexture(&ncScene.getTexture());
    ncPreview();
    
    // Debug draw
    
    webcam.draw(220, 10);
    debugDraw();
    
    
}
void ofApp::debugDraw(){
    
    ofPushMatrix();
    //ofTranslate(20, 30);
    //ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), 0, 0);
    
    //grayscale.draw(0, 0, 320, 240);
    //background.draw(0, 270, 320, 240);
    //difference.draw(0, 520, 320, 240);
    //contour.draw(0, 520, 320, 240);

    ofColor c(255, 255, 255);
    ofLog(OF_LOG_NOTICE, "the number of blobs is %d", contour.nBlobs);
    
    for(int i = 0; i < contour.nBlobs; i++) {
        ofRectangle r = contour.blobs.at(i).boundingRect;
        //ofLog(OF_LOG_NOTICE, "blobRectangle is %f", r.width);
        //r.x += 320; r.y += 240;
        c.setHsb(10 * i,200, 200);
        ofSetColor(c);
        ofDrawRectangle(r);
    }
    ofPopMatrix();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == 'f') ofToggleFullscreen();
    if(key == 's') ncExportImage();
    if(key == 'r') ncRotate = !ncRotate;
    if(key == 'p') ncPaused = !ncPaused;
    
    if(key == ' ') background = grayscale;
    if(key == OF_KEY_DOWN) threshold--;
    if(key == OF_KEY_UP) threshold++;
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::ncSetup(string _name, int _width, int _height){
    
    // Application state
    
    ncPaused = false;
    ncRotate = true;
    ncCamera.set(0, 0, 0);
    
    // FBO scale - Change this
    // for hi-res export
    
    ncScale = 1;
    
    // Allocate FBO
    
    ofFbo::Settings s;
    s.width = _width * ncScale;
    s.height = _height * ncScale;
    s.useDepth = true;
    ncScene.allocate(s);
    
    // Syphon server
    
    ncServer.setName(_name);
    
}

//--------------------------------------------------------------
void ofApp::ncPreview(){
    
    // Set size & position
    // of the preview window
    
    float aspect = ncScene.getHeight() / ncScene.getWidth();
    int width = ofGetWidth() * 0.9;
    int height = width * aspect;
    int x = (ofGetWidth() - width) * 0.5;
    int y = (ofGetHeight() - height) * 0.5;
    ncScene.draw(x, y, width, height);
    
    // Draw outline so we can
    // see the preview area
    
    ofPushStyle();
    ofNoFill();
    ofSetLineWidth(1);
    ofSetColor(255, 255, 255, 100);
    ofDrawRectangle(x, y, width, height);
    ofPopStyle();
    
    // Draw GUI
    
    gui.draw();

}

//--------------------------------------------------------------
void ofApp::ncExportImage(){
    
    ofImage hires;
    ncScene.getTexture().readToPixels(hires);
    hires.save("snapshot-"+ ofGetTimestampString() +".png", OF_IMAGE_QUALITY_BEST);
    
}

