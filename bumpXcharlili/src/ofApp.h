#pragma once

#include "ofMain.h"
#include "ncVideoGrabber.h"
#include "ofxSyphon.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{
public:
		
    void setup();
    void update();
    void draw();
    void debugDraw();
    void drawBlobs();
    void getBlobs();
		
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    //-------------------------------------------
    // My app...
    
    ncVideoGrabber webcam;
    ofPixels pxs;
    ofImage image;
    ofxCvColorImage rgb;
    ofxCvGrayscaleImage grayscale;
    ofxCvGrayscaleImage background;
    ofxCvGrayscaleImage difference;
    ofxCvContourFinder contour;
    float threshold;
    
    
    
    //-------------------------------------------
    // GUI

    ofxPanel gui;
    ofParameterGroup parameters;
    
    //-------------------------------------------
    // The template app draws everything to an FBO
    // so we can easily export to Syphon and/or save
    // hi-res images (with transparent background)
    
    bool ncRotate;
    bool ncPaused;
    int ncScale;
    
    ofShader shader;
    ofPlanePrimitive plane;
    
    ofImage backgroundImage;
    ofImage foregroundImage;
    ofImage brushImage;
    ofFbo maskFbo;
    ofFbo fbo;
    bool bBrushDown;
    
    ofxSyphonServer ncServer;
    ofVec3f ncCamera;
    ofFbo ncScene;
    
    void ncSetup(string name, int width, int height);
    void ncPreview();
    void ncExportImage();

};
