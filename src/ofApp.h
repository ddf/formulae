#pragma once

#include "ofMain.h"
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

	std::vector<VarViz*> mVars;
};
