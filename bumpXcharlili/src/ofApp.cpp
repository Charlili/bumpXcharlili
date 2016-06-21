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
    backgroundImage.load("bg3.mov");
    backgroundImage.play();
    foregroundImage.load("bg2.gif");
    brushImage.load("brush.png");
    WIDTH = ncScene.getWidth();
    HEIGHT = ncScene.getHeight();
    maskFbo.allocate(WIDTH+1, HEIGHT+1);
    fbo.allocate(WIDTH+1, HEIGHT+1);
    maskFbo.begin();
    ofClear(0,0,0,255);
    maskFbo.end();
    
    fbo.begin();
    ofClear(0,0,0,255);
    fbo.end();
    
    
    nTri = 100; //The number of the triangles
    nVert= nTri * 3; //The number of the vertices
    float Rad = 800; //The sphere's radius
    float rad = 250; //Maximal triangle's "radius"
    //(formally, it's the maximal coordinates'
    //deviation from the triangle's center)
    //Fill the vertices array
    vertices.resize( nVert ); //Set the array size
    for (int i=0; i<nTri; i++) { //Scan all the triangles
        //Generate the center of the triangle
        //as a random point on the sphere
        //Take the random point from
        //cube [-1,1]x[-1,1]x[-1,1]
        int x = WIDTH * 0.3;
        int y = HEIGHT* 0.5;
        
        ofPoint center( ofRandom( -1, 1 ),
                       ofRandom( -1, 1 ),
                       ofRandom( -1, 1 ) );
        center.normalize(); //Normalize vector's length to 1
        center *= Rad; //Now the center vector has
        
        //center.x = x;
        //length Rad
        //Generate the triangle's vertices
        //as the center plus random point from
        //[-rad, rad]x[-rad, rad]x[-rad, rad]
        for (int j=0; j<3; j++) {
            vertices[ i*3 + j ] = center + ofPoint( x + ofRandom( -rad, rad ),
                             y + ofRandom( -rad, rad ),
                             ofRandom( -rad, rad ) );
        }
    }
    //Fill the array of triangles' colors
    colors.resize( nTri );
    for (int i=0; i<nTri; i++) {
        //Take a random color from black to red
        colors[i] = ofColor( ofRandom( 0, 255 ), 0, 0 );
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
    backgroundImage.update();
    getBlobs();
    
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
    float speed = ofMap( mouseY, 0, HEIGHT, 0, 5.0 );
    phase += speed * dt;
    distortAmount = ofMap( mouseX, 0, WIDTH, 0, 1.0 );
    
    
    
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
    
    backgroundImage.draw(0,-HEIGHT,WIDTH*3,HEIGHT*3);

    
    
    //get center screen
    int x = ncScene.getWidth() * 0.5;
    int y = ncScene.getHeight() * 0.5;
    
    int newWidth = backgroundImage.getWidth() * HEIGHT / backgroundImage.getHeight();
    //drawBlobs();
    maskFbo.begin();
    ofClear(0, 0, 0, 0);
    ofPushMatrix();
    ofEnableDepthTest();
    ofEnableAlphaBlending();
    
    float time = ofGetElapsedTimef(); //Get time in seconds
    float angle = time * 10; //Compute angle. We rotate at speed
    //10 degrees per second
    ofTranslate(x, y);
    ofRotate( angle, 0, 1, 0 );
    ofTranslate(-x, -y);
    shader_deform.begin();
    ofSetColor( ofColor(255,255,255) );
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
    backgroundImage.draw(0,-HEIGHT/2,WIDTH,HEIGHT*2);
    //ofTranslate(-x, -y);
    shader_mask.end();
    //backgroundImage.draw(0,-HEIGHT/2,WIDTH,HEIGHT*2);
    ofDisableAlphaBlending();
    fbo.end();
    if(fbo.isAllocated())fbo.draw(0,0);
    
    debugDraw();
    
    //maskFbo.draw(0,0);
    //backgroundImage.draw(0, 0,WIDTH,HEIGHT);
    ofTranslate(x, y);
    
    ofRotateY(ncCamera.x);
    ofRotateZ(ncCamera.y);
    
    //todo:make foreground-image gif-loop
    //backgroundImage.draw(0, 0,WIDTH,ratio);
    //foregroundImage.draw(0,0,WIDTH,HEIGHT);
    
    // App stuff
    
    /*ofNoFill();
    ofSetLineWidth(10);
    ofDrawBox(75);
    ofDrawBox(150);
    ofDrawBox(300);
    ofDrawBox(600);
    ofDrawBox(1200);
    */
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
    maskFbo.begin();
    int scale = WIDTH / webcam.getTexture().getWidth();
    
    for(int i = 0; i < contour.nBlobs; i++) {
        //rectangle at blob
        ofRectangle r = contour.blobs.at(i).boundingRect;
        r.x = ofMap(r.x, 0, (float)webcam.getTexture().getWidth(), 0, (float)WIDTH);
        r.y = ofMap(r.y, 0, (float)webcam.getTexture().getHeight(), 0, (float)HEIGHT);
        r.width *= scale;
        r.height *= scale;
        
        //c.setHsb(10 * i,200, 200);
        //ofSetColor(c);
        
        float percentX = r.x / (float)WIDTH;
        percentX = ofClamp(percentX, 0, 1);
        ofColor colorLeft = ofColor::magenta;
        ofColor colorRight = ofColor::cyan;
        ofColor colorMix = colorLeft.getLerped(colorRight, percentX);
        ofSetColor(colorMix);
        int brushImageSize = r.width*10;
        if(r.width < r.height)brushImageSize = r.height*10;
        //brushImage.draw(r.x, r.y, brushImageSize, brushImageSize);
        ofDrawRectangle(r);
    }
    maskFbo.end();
}
void ofApp::debugDraw(){
    
    ofPushMatrix();
    
    //grayscale.draw(0, 520, 640, 240);
    //background.draw(0, 270, 640, 240);
    //difference.draw(640, 520, 640, 240);
    //contour.draw(0, 520, 640, 240);
    
    //ofLog(OF_LOG_NOTICE, "the number of blobs is %d", contour.nBlobs);
    ofColor c(255, 255, 255);
     int scale = WIDTH / webcam.getTexture().getWidth();
    for(int i = 0; i < contour.nBlobs -1; i++) {
    //if(contour.nBlobs>0){
        ofRectangle r = contour.blobs.at(i).boundingRect;
        //r.x += 0;
        //r.y += 520;
        r.x = ofMap(r.x, 0, (float)webcam.getTexture().getWidth(), 0, (float)WIDTH);
        r.y = ofMap(r.y, 0, (float)webcam.getTexture().getHeight(), 0, (float)HEIGHT);
        r.width *= scale;
        r.height *= scale;
        //c.setHsb(10 * i,200, 200);
        //ofSetColor(c);
        
        float percentX = r.x / (float)WIDTH;
        percentX = ofClamp(percentX, 0, 1);
        ofColor colorLeft = ofColor::magenta;
        ofColor colorRight = ofColor::cyan;
        ofColor colorMix = colorLeft.getLerped(colorRight, percentX);
        ofSetColor(colorMix);
        int brushImageSize = r.width*10;
        if(r.width < r.height)brushImageSize = r.height*10;
        //brushImage.draw(r.x, r.y, brushImageSize, brushImageSize);
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
    bBrushDown = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    bBrushDown = false;
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

