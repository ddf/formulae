#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "Program.h"
#include "VarViz.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

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
		

		virtual void audioOut(ofSoundBuffer& output) override;

private:
	ofSoundStreamSettings mSoundSettings;
	ofMutex mMutex;
	ofSoundBuffer mOutput;
	size_t mOutputBegin;
	Program* mProgram;
	unsigned mBitDepth;
	unsigned mTempo;
	Program::Value mTick;

	ofxGuiGroup mGUI;
	ofParameter<Program::Value> V0, V1, V2, V3, V4, V5, V6, V7;

	std::vector<VarViz*> mVars;
};
