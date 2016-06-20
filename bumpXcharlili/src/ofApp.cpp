#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    }else{
        shader.load("shadersGL2/shader");
    }
#endif
    
    ofBackground(0);
    ncSetup("BUMP-MYSKETCH", 3600, 1080);
    threshold = 33;
    ncRotate = false;
    
    
    //blobDetection
    webcam.setup(320, 240);
    rgb.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    rgb.mirror(false, true);
    grayscale.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    background.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    difference.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    //shader
    float planeScale = 3;
    int planeWidth = ofGetWidth() * planeScale;
    int planeHeight = ofGetHeight() * planeScale;
    int planeGridSize = 50;
    int planeColums = planeWidth / planeGridSize;
    int planeRows = planeHeight / planeGridSize;
    
    plane.set(planeWidth, planeHeight, planeColums, planeRows, OF_PRIMITIVE_TRIANGLES);
    
    // GUI
    //parameters.add(myVariable.set("Some boolean", true));
    //parameters.add(myClass.parameters);
    gui.setup(parameters);
    
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if(ncPaused) return;
    ofSetWindowTitle("FPS: "+ ofToString(ofGetFrameRate()));

    
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
    
    webcam.getTexture().readToPixels(pxs);
    rgb.setFromPixels(pxs);
    grayscale = rgb;
    difference.absDiff(background, grayscale);
    difference.threshold(threshold);
    
    contour.findContours(difference, 10, 510, 20, false,true);
    
    ofPixels gray = grayscale.getPixels();
    ofPixels bg = background.getPixels();
    for(int i=0; i<webcam.getTexture().getWidth()*webcam.getTexture().getHeight(); i++){
        
        bg[i] *= 0.9;
        bg[i] += gray[i]*0.1;
        
    }
    background = bg;
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
    
    drawBlobs();
    
    //change color from mouse position
    float percentX = mouseX / (float)ofGetWidth();
    percentX = ofClamp(percentX, 0, 1);
    ofColor colorLeft = ofColor::magenta;
    ofColor colorRight = ofColor::cyan;
    ofColor colorMix = colorLeft.getLerped(colorRight, percentX);
    ofSetColor(colorMix);
    
    //start shader with sinus animation
    shader.begin();
    shader.setUniform1f("time", ofGetElapsedTimef());
    
    //get center screen
    int x = ncScene.getWidth() * 0.5;
    int y = ncScene.getHeight() * 0.5;
    
    ofTranslate(x, y);
    ofRotateY(ncCamera.x);
    ofRotateZ(ncCamera.y);
    
    // the mouse/touch Y position changes the rotation of the plane.
    float percentY = mouseY / (float)ofGetHeight();
    float rotation = ofMap(percentY, 0, 1, -60, 60, true) + 60;
    //ofRotate(rotation, 1, 0, 0);
    plane.drawWireframe();
    shader.end();
    
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
    
    //DebugDraw
    webcam.draw(220, 10);
    //debugDraw();
    
}
void ofApp::drawBlobs(){
    ofPushMatrix();
    ofColor c(255, 255, 255);
    //ofLog(OF_LOG_NOTICE, "the number of blobs is %d", contour.nBlobs);
    int scale = ncScene.getWidth() / webcam.getTexture().getWidth();
    for(int i = 0; i < contour.nBlobs; i++) {
        ofRectangle r = contour.blobs.at(i).boundingRect;
        r.x *= scale;
        r.y *= scale;
        r.width *= scale;
        r.height *= scale;
        c.setHsb(10 * i,200, 200);
        ofSetColor(c);
        ofDrawRectangle(r);
    }
    ofPopMatrix();
}
void ofApp::debugDraw(){
    
    ofPushMatrix();
    grayscale.draw(0, 520, 640, 240);
    //background.draw(0, 270, 640, 240);
    //difference.draw(640, 520, 640, 240);
    //contour.draw(0, 520, 640, 240);
    
    ofColor c(255, 255, 255);
    for(int i = 0; i < contour.nBlobs -1; i++) {
        ofRectangle r = contour.blobs.at(i).boundingRect;
        r.x += 0;
        r.y += 520;
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
    ofLog(OF_LOG_NOTICE, "threshold = %f", threshold);
    
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

