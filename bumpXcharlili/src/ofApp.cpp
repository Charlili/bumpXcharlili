#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
#ifdef TARGET_OPENGLES
    shader_deform.load("shadersES2/shader_deform");
#else
    if(ofIsGLProgrammableRenderer()){
        shader_deform.load("shadersGL3/shader_deform");
    }else{
        shader_deform.load("shadersGL2/shader_deform");
        shader_mask.load("shadersGL2/shader_mask");
        
    }
#endif
    
    ofBackground(0);
    ncSetup("BUMP-CHARLILI", 3600, 1080);
    threshold = 33;
    ncRotate = false;
    ofEnableAlphaBlending();
    
    //blobDetection
    webcam.setup(320, 240);
    rgb.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    rgb.mirror(false, true);
    grayscale.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    background.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    difference.allocate(webcam.getTexture().getWidth(), webcam.getTexture().getHeight());
    
    //shader_deform with images & mask
    foregroundImage.load("bg4.png");
    vignetteImage.load("bg5.png");
    
    
    WIDTH = ncScene.getWidth();
    HEIGHT = ncScene.getHeight();
    maskFbo.allocate(WIDTH, HEIGHT);
    fbo.allocate(WIDTH, HEIGHT);
    maskFbo.begin();
    ofClear(0,0,0,255);
    maskFbo.end();
    blobFbo.allocate(WIDTH,HEIGHT);
    blobFbo.begin();
    ofClear(0,0,0,0);
    blobFbo.end();
    
    fbo.begin();
    ofClear(0,0,0,255);
    fbo.end();
    psBlend.setup(WIDTH, HEIGHT);
    psLijntjes.setup(WIDTH,HEIGHT);
    blendMode = 3;
    
    nTri = 100;
    nVert= nTri * 3;
    float Rad = 800; //sphere's radius
    float rad = 250; //triangle's "radius"
    vertices.resize( nVert ); //Set the array size
    for (int i=0; i<nTri; i++) {
        int x = WIDTH * 0.3;
        int y = HEIGHT* 0.5;
        ofPoint center( ofRandom( -1, 1 ),
                       ofRandom( -1, 1 ),
                       ofRandom( -1, 1 ) );
        center.normalize();
        center *= Rad;
        for (int j=0; j<3; j++) {
            vertices[ i*3 + j ] = center + ofPoint( x + ofRandom( -rad, rad ),
                                                   y + ofRandom( -rad, rad ),
                                                   ofRandom( -rad, rad ) );
        }
    }
    float time0 = 0;
    float phase = 0;
    float distortAmount = 0;
    
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
    
    psBlend.begin();
    vignetteImage.draw(0,0);
    psBlend.end();
    
    psLijntjes.begin();
    psBlend.draw(fbo.getTexture(), blendMode);
    psLijntjes.end();
    
    // Simple 'camera'
    // Just rotates stuff
    
    if(ncRotate){
        float dX = (float)ofGetMouseX()/(float)ofGetWidth() * 360;
        float dY = (float)ofGetMouseY()/(float)HEIGHT * 360;
        ncCamera.x += (dX - ncCamera.x) * 0.05;
        ncCamera.y += (dY - ncCamera.y) * 0.05;
    }
    
    //Compute dt
    float time = ofGetElapsedTimef();
    float dt = ofClamp( time - time0, 0, 0.1 );
    time0 = time;
    float speed = ofMap( mouseY, 0, HEIGHT, 0, 2.0 );
    phase += speed * dt;
    distortAmount = ofMap( mouseX, 0, WIDTH, 0.5, 1.0 );
    
}
void ofApp::getBlobs(){
    
    webcam.getTexture().readToPixels(pxs);
    rgb.setFromPixels(pxs);
    grayscale = rgb;
    difference.absDiff(background, grayscale);
    difference.threshold(threshold);
    
    contour.findContours(difference, 10, 510, 40, false,true);
    
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
    
    vignetteImage.draw(0,0);
    
    //get center screen
    int x = ncScene.getWidth() * 0.5;
    int y = ncScene.getHeight() * 0.5;
    
    //drawBlobs();
    maskFbo.begin();
    ofClear(0, 0, 0, 0);
    ofPushMatrix();
    ofEnableDepthTest();
    ofEnableAlphaBlending();
    
    float time = ofGetElapsedTimef(); //Get time in seconds
    float angle = time * 10; //Compute angle. We rotate at speed
    ofTranslate(x, y);
    ofRotate( angle, 0, 1, 0 );
    ofTranslate(-x, -y);
    shader_deform.begin();
    shader_deform.setUniform1f("phase", phase );
    shader_deform.setUniform1f("distortAmount", distortAmount );
    for (int i=0; i<nTri; i++) {
        ofSetColor( ofColor(255,255,255) ); //Set color
        ofDrawTriangle( vertices[ i*3 ],
                       vertices[ i*3 + 1 ],
                       vertices[ i*3 + 2 ] ); //Draw triangle
    }
    shader_deform.end();
    ofPopMatrix();
    ofDisableAlphaBlending();
    ofDisableDepthTest();
    maskFbo.end();
    
    fbo.begin();
    ofClear(0, 0, 0, 0);
    ofEnableAlphaBlending();
    shader_mask.begin();
    if(maskFbo.isAllocated())shader_mask.setUniformTexture("maskTex", maskFbo.getTexture(), 1 );
    foregroundImage.draw(0,0);
    
    shader_mask.end();
    ofDisableAlphaBlending();
    fbo.end();
    
    if(fbo.isAllocated()) psBlend.draw(fbo.getTexture(), 21);//2 //21
    
    drawBlobs();
    psLijntjes.draw(blobFbo.getTexture(),3);//3 //3
    
    ofTranslate(x, y);
    ofRotateY(ncCamera.x);
    ofRotateZ(ncCamera.y);
    
    
    // Close drawing to FBO, export texture
    // to Syphon and preview our scene
    
    ofDisableAlphaBlending();
    ofDisableDepthTest();
    
    
    ofPopMatrix();
    ncScene.end();
    ncServer.publishTexture(&ncScene.getTexture());
    ncPreview();
    
    webcam.draw(220, 10);
}
void ofApp::drawBlobs(){
    
    ofPushMatrix();
    blobFbo.begin();
    ofClear(0, 0, 0, 0);
    
    int scale = WIDTH / webcam.getTexture().getWidth();
    for(int i = 0; i < contour.nBlobs -1; i++) {
        if(contour.nBlobs>0){
            ofRectangle r = contour.blobs.at(i).boundingRect;
            
            r.x = ofMap(r.x, 0, (float)webcam.getTexture().getWidth(), 0, (float)WIDTH);
            r.y = ofMap(r.y, 0, (float)webcam.getTexture().getHeight(), 0, (float)HEIGHT);
            r.width *= scale;
            r.height *= scale;
            
            float percentX = r.x / (float)WIDTH;
            percentX = ofClamp(percentX, 0, 1);
            ofColor colorLeft = ofColor::magenta;
            ofColor colorRight = ofColor::cyan;
            ofColor colorMix = colorLeft.getLerped(colorRight, percentX);
            ofSetColor(colorMix);
            ofDrawRectangle(r.x, 0, r.width, HEIGHT);
        }
    }

    blobFbo.end();
    ofPopMatrix();
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key == 'f') ofToggleFullscreen();
    if(key == 's') ncExportImage();
    if(key == 'r') ncRotate = !ncRotate;
    if(key == 'p') ncPaused = !ncPaused;
    
    if(key == ' ') background = grayscale;
    if(key == OF_KEY_DOWN) {threshold--;ofLog(OF_LOG_NOTICE, "threshold = %f", threshold);}
    if(key == OF_KEY_UP) {threshold++;ofLog(OF_LOG_NOTICE, "threshold = %f", threshold);}
    
    
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

