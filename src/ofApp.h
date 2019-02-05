#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"

#include "MenuGUI.h"

#include "Program.h"
#include "ProgramGUI.h"
#include "VarViz.h"

class ofApp : public ofBaseApp, public ofxMidiListener {

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
		

		virtual void audioOut(ofSoundBuffer& output) override;
		virtual void newMidiMessage(ofxMidiMessage& message) override;

private:
	void loadProgram(ofXml programSettings);
	void closeProgram();
	void toggleProgramState();
	void paramChanged(ofAbstractParameter& param);
	void programSelected(int& idx);

	enum State
	{
		kStateMenu,
		kStateProgramStopped,
		kStateProgramRunning,
	} mState;

	ofSoundStreamSettings mSoundSettings;
	// full settings.xml file
	ofXml mAppSettings;
	std::vector<ofXml> mProgramList;

	ofMutex mOutputMutex;
	ofSoundBuffer mOutput;
	size_t mOutputRead;
	// sound buffer we copy mOutput to before rendering it,
	// to prevent rendering from holding up the audio thread
	ofSoundBuffer mOutputRender;

	MenuGUI mMenu;

	Program* mProgram;
	ofMutex mProgramMutex;
	std::string mCode;
	unsigned mBitDepth;
	unsigned mTempo;
	Program::Value mTick;

	ProgramGUI mProgramGUI;
	ofParameter<Program::Value> V0, V1, V2, V3, V4, V5, V6, V7;
	std::map<std::string, ofParameter<Program::Value>*> mVC = 
	{
		{ "V0", &V0 },
		{ "V1", &V1 },
		{ "V2", &V2 },
		{ "V3", &V3 },
		{ "V4", &V4 },
		{ "V5", &V5 },
		{ "V6", &V6 },
		{ "V7", &V7 }
	};
	// UI currently being controlled by the keyboard, can be nullptr
	ofxBaseGui* mKeyUI;
	// V params currently in use
	ofParameterGroup mParams;

	std::vector<VarViz*> mVars;

	ofxMidiIn mMidiIn;
	ofxMidiOut mMidiOut;
	// associates a parameter with a CC value and Channel, which are combined into a single int with the channel in the high-bits
	std::map<int, ofParameter<Program::Value>*> mMidiMap;
};
