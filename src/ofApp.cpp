#include "ofApp.h"

const size_t kOutputChannels = 2;
const size_t kSampleRate = 44100;
const size_t kBufferSize = 512;

std::map<std::string, VarVizType> kVizType = {
	{ "value", kVizTypeValue },
	{ "wave", kVizTypeWave },
	{ "bars", kVizTypeBars },
	{ "blocks", kVizTypeBlocks },
	{ "scatter", kVizTypeScatter }
};

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);

	mOutput.setNumChannels(kOutputChannels);
	mOutput.setSampleRate(kSampleRate);
	mOutput.resize(kBufferSize*8);

	mProgram = nullptr;
	mKeyUI = nullptr;
	mState = kStateMenu;
	
	mSoundSettings.numOutputChannels = kOutputChannels;
	mSoundSettings.sampleRate = kSampleRate;
	mSoundSettings.bufferSize = kBufferSize;
	mSoundSettings.setApi(ofSoundDevice::MS_DS);
	mSoundSettings.setOutListener(this);
	ofSoundStreamSetup(mSoundSettings);

	mAppSettings.load("settings.xml");

#if defined(NDEBUG)
	ofSetFullscreen(mAppSettings.getFirstChild().getAttribute("fullscreen").getBoolValue());
#endif

	mMenuText = "Choose a program:\n\n";
	int id = 1;
	for (auto child : mAppSettings.getFirstChild().getChildren("program"))
	{
		mProgramList.push_back(child);
		mMenuText += "\t" + std::to_string(id) + ". " + child.getAttribute("name").getValue() + "\n";
		++id;
	}

	mMenu.setup(mMenuText, ofGetWidth(), 20);
	mMenu.setPosition(10, 10);

	ofAddListener(mProgramGUI.closePressedE, this, &ofApp::closeProgram);
}

//--------------------------------------------------------------
void ofApp::loadProgram(ofXml programSettings)
{
	mCode = programSettings.getChild("code").getValue();
	mTick = 0;
	mTempo = programSettings.getAttribute("tempo").getUintValue();
	mBitDepth = programSettings.getAttribute("bits").getUintValue();
	Program::Value memorySize = programSettings.getAttribute("memory").getUintValue();

	ofxBaseGui::setDefaultBorderColor(ofColor(255));
	ofxBaseGui::setDefaultBackgroundColor(0);
	ofxBaseGui::setDefaultHeaderBackgroundColor(0);
	ofxBaseGui::setDefaultHeight(40);
	ofxBaseGui::setDefaultTextPadding(10);

	std::string programName = programSettings.getAttribute("name").getValue();
	mProgramGUI.setup(programName, programName + ".xml", 0, 0);
	mProgramGUI.setShape(0, 0, ofGetWidth(), ofGetHeight());
	mProgramGUI.setHeaderBackgroundColor(0);
	mProgramGUI.setBackgroundColor(0);

	ofXml interfaceSettings = programSettings.getChild("interface");
	if (interfaceSettings)
	{
		// set size to reference size for initial placement
		const float w = interfaceSettings.getAttribute("w").getFloatValue();
		const float h = interfaceSettings.getAttribute("h").getFloatValue();
		mProgramGUI.setSize(w, h);

		// setup all the controls
		for (auto child : interfaceSettings.getChildren())
		{
			auto element = child.getName();
			auto name = child.getAttribute("name").getValue();
			if (element == "control")
			{
				auto VC = mVC[child.getAttribute("target").getValue()];
				VC->setMin(child.getAttribute("min").getUintValue());
				VC->setMax(child.getAttribute("max").getUintValue());
				VC->set(name, child.getAttribute("value").getUintValue());

				mProgramGUI.add(*VC);
			}
			else if (element == "viz")
			{
				auto source = child.getAttribute("source").getValue();
				if (!source.empty())
				{
					Program::Value var = 0;
					if (source[0] == '@')
					{
						source = source.substr(1);
						var = std::stoull(source);
					}
					else
					{
						var = Program::GetAddress(source[0], memorySize);
					}
					auto vizType = kVizType[child.getAttribute("type").getValue()];
					auto buffer = child.getAttribute("buffer").getUintValue();
					auto rate = std::max(child.getAttribute("rate").getUintValue(), 1U);
					auto range = child.getAttribute("range").getUintValue();
					auto viz = new VarViz(name, var, vizType, buffer, rate, range);
					auto columns = child.getAttribute("columns");
					if (columns)
					{
						viz->setColumns(std::max(columns.getUintValue(), 1U));
					}

					mProgramGUI.add(viz);
					mVars.push_back(viz);
				}
			}

			ofxBaseGui* ui = mProgramGUI.getControl(name);
			auto shape = child.getChild("shape");
			if (ui != nullptr && shape)
			{
				float x = shape.getAttribute("x").getFloatValue();
				float y = shape.getAttribute("y").getFloatValue();
				float w = shape.getAttribute("w").getFloatValue();
				float h = shape.getAttribute("h").getFloatValue();

				ui->setShape(x, y, w, h);
			}
		}		

		// expand to fullscreen
		mProgramGUI.setSize(ofGetWidth(), ofGetHeight());
		if (interfaceSettings.getAttribute("maximize").getBoolValue())
		{
			mProgramGUI.maximize();
		}
		else
		{
			mProgramGUI.minimize();
		}
	}	
	else
	{
		mProgramGUI.minimize();
	}

	Program::CompileError error;
	int errorPosition;
	mProgram = Program::Compile(mCode.c_str(), memorySize, error, errorPosition);

	mState = kStateProgram;
}

void ofApp::closeProgram()
{
	if (mState == kStateProgram)
	{
		mMutex.lock();
		delete mProgram;
		mProgram = nullptr;
		// clear the gui, this will delete all UI, including our mVars
		mProgramGUI.teardown();
		mKeyUI = nullptr;
		mVars.clear();
		mMutex.unlock();

		mState = kStateMenu;
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(0);

	const size_t frames = mOutput.size() / 2;
	const int cols = 32;
	const int rows = frames / cols;
	const float hw = ofGetWidth() / 2;
	const float hh = (float)ofGetHeight() / 2;
	const float w = hw / cols;	
	const float h = hh * 2 / rows;

	ofSetRectMode(OF_RECTMODE_CORNER);
	ofFill();

	if (mState == kStateProgram)
	{
		mMutex.lock();
		size_t outputBegin = mTick < frames ? 0 : (mTick - frames) % frames;
		for (size_t i = 0; i < frames; ++i)
		{
			int x = w * (i%cols);
			int y = h * (i / cols);
			size_t f = (outputBegin + i) % frames;
			ofSetColor((mOutput.getSample(f, 0) + 1)*127.5f);
			ofDrawRectangle(x, y, w, h);
			x += hw;
			ofSetColor((mOutput.getSample(f, 1) + 1)*127.5f);
			ofDrawRectangle(x, y, w, h);
		}

		mProgramGUI.draw();
		mMutex.unlock();
	}
	else
	{
		mMenu.draw();
	}
}

void ofApp::exit()
{
	closeProgram();
	ofSoundStreamClose();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (mState == kStateMenu)
	{
		int pidx = key - '1';
		if (pidx >= 0 && pidx < mProgramList.size())
		{
			loadProgram(mProgramList[pidx]);
		}
	}
	else if (mState == kStateProgram)
	{
		if (key == 'q')
		{
			closeProgram();
		}
		else if (key == '+' && mProgramGUI.isMinimized())
		{
			mProgramGUI.maximize();
		}
		else if(key == '-' && !mProgramGUI.isMinimized())
		{
			mProgramGUI.minimize();
		}
		else if (mKeyUI != nullptr && (key >= OF_KEY_LEFT && key <= OF_KEY_DOWN) || (key >= '0' && key <= '9'))
		{
			auto param = mKeyUI->getParameter().cast<Program::Value>();
			if ((key == OF_KEY_RIGHT || key == OF_KEY_UP) && param.get() < param.getMax())
			{
				param.set(param.get() + 1);
			}
			else if ((key == OF_KEY_LEFT || key == OF_KEY_DOWN) && param.get() > param.getMin())
			{
				param.set(param.get() - 1);
			}
			else
			{
				Program::Value val = key - '0';
				if (val >= param.getMin() && val <= param.getMax())
				{
					param.set(val);
				}
			}
		}
		else if (std::isalpha(key))
		{
			std::string name(1, key);
			mKeyUI = mProgramGUI.getControl(name);
		}
	}
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
void ofApp::windowResized(int w, int h)
{
	if (mState == kStateProgram)
	{
		mProgramGUI.setSize(w, h);
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::audioOut(ofSoundBuffer& output)
{
	const size_t bufferSize = output.size();
	const size_t nChannels = output.getNumChannels();

	mMutex.lock();
	if (mProgram != nullptr)
	{		
		const Program::Value range = (Program::Value)1 << mBitDepth;
		const float mdenom = mSoundSettings.sampleRate / 1000.0f;
		const float qdenom = (mSoundSettings.sampleRate / (mTempo / 60.0f)) / 128.0f;

		mProgram->Set('w', range);
		mProgram->Set('~', (Program::Value)mSoundSettings.sampleRate);

		Program::Value results[kOutputChannels];
		Program::RuntimeError error;		
		for (size_t f = 0; f < bufferSize; f += nChannels)
		{
			mProgram->Set('t', mTick);
			mProgram->Set('m', (Program::Value)round(mTick / mdenom));
			mProgram->Set('q', (Program::Value)round(mTick / qdenom));
			mProgram->SetVC(0, V0);
			mProgram->SetVC(1, V1);
			mProgram->SetVC(2, V2);
			mProgram->SetVC(3, V3);
			mProgram->SetVC(4, V4);
			mProgram->SetVC(5, V5);
			mProgram->SetVC(6, V6);
			mProgram->SetVC(7, V7);
			results[0] = 0;
			results[1] = 0;
			error = mProgram->Run(results, 2);
			output[f] = -1.0f + 2.0f*((float)(results[0] % range) / (range - 1));
			output[f + 1] = -1.0f + 2.0f*((float)(results[1] % range) / (range - 1));
			for (size_t i = 0; i < mVars.size(); ++i)
			{
				auto  var = mVars[i];
				var->push(mProgram->Peek(var->getVar()));
			}
			++mTick;
		}
	}

	size_t writeBegin = (mTick*nChannels - bufferSize) % mOutput.size();
	const size_t outSize = mOutput.size();
	for (size_t f = 0; f < bufferSize; f += nChannels)
	{
		size_t i = (writeBegin + f) % outSize;
		mOutput[i] = output[f];
		mOutput[i + 1] = output[f + 1];
	}
	mMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
