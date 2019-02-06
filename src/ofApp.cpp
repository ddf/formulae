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
	ofSetEscapeQuitsApp(false);

	mOutput.setNumChannels(kOutputChannels);
	mOutput.setSampleRate(kSampleRate);
	mOutput.resize(kBufferSize*8);
	mOutputRead = 0;
	mOutputRender.setNumChannels(kOutputChannels);
	mOutputRender.setSampleRate(kSampleRate);
	mOutputRender.resize(kBufferSize*8);

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

	auto midi = mAppSettings.getFirstChild().getChild("midi");
	if (midi)
	{
		auto device = midi.getAttribute("device");
		if (device)
		{
			std::string deviceName = device.getValue();
			int count = mMidiIn.getNumInPorts();
			for (int p = 0; p < count; ++p)
			{
				if (mMidiIn.getInPortName(p).find(deviceName) != std::string::npos)
				{
					if (mMidiIn.openPort(p))
					{
						break;
					}
				}
			}

			count = mMidiOut.getNumOutPorts();
			for (int p = 0; p < count; ++p)
			{
				if (mMidiOut.getOutPortName(p).find(deviceName) != std::string::npos)
				{
					if (mMidiOut.openPort(p))
					{
						break;
					}
				}
			}
		}
		else
		{
			// open first available
			mMidiIn.openPort();
			mMidiOut.openPort();
		}

		if (mMidiIn.isOpen())
		{
			mMidiIn.addListener(this);
		}
	}

	ofxBaseGui::setDefaultWidth(250);
	ofxBaseGui::setDefaultHeight(40);
	ofxBaseGui::setDefaultTextPadding(10);

	const std::string prompt = mAppSettings.getFirstChild().getChild("prompt").getValue();
	mMenu.setup(prompt, "menu.xml", 0, 0);

	ofxBaseGui::setDefaultHeight(20);

	int id = 1;
	for (auto child : mAppSettings.getFirstChild().getChildren("program"))
	{
		mProgramList.push_back(child);
		std::string label(std::to_string(id));
		label += ". " + child.getAttribute("name").getValue();
		mMenu.add(label);
		++id;
	}

	mMenu.maximize();

	ofAddListener(mMenu.buttonClickedE, this, &ofApp::programSelected);
	ofAddListener(mProgramGUI.closePressedE, this, &ofApp::closeProgram);
	ofAddListener(mProgramGUI.transportPressedE, this, &ofApp::toggleProgramState);
	ofAddListener(mParams.parameterChangedE(), this, &ofApp::paramChanged);
}

//--------------------------------------------------------------
void ofApp::loadProgram(ofXml programSettings)
{
	mCode = programSettings.getChild("code").getValue();
	mTick = 0;
	mTempo = programSettings.getAttribute("tempo").getUintValue();
	mBitDepth = programSettings.getAttribute("bits").getUintValue();
	Program::Value memorySize = programSettings.getAttribute("memory").getUintValue();

	ofxBaseGui::setDefaultBorderColor(255);
	ofxBaseGui::setDefaultBackgroundColor(0);
	ofxBaseGui::setDefaultHeaderBackgroundColor(0);
	ofxBaseGui::setDefaultHeight(40);
	ofxBaseGui::setDefaultTextPadding(10);

	std::string programName = programSettings.getAttribute("name").getValue();
	mProgramGUI.setup(programName, mCode, programName + ".xml", 0, 0);
	mProgramGUI.setShape(0, 0, ofGetWidth(), ofGetHeight());
	mProgramGUI.setHeaderBackgroundColor(0);
	mProgramGUI.setBackgroundColor(64);

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
				auto pair = mVC.find(child.getAttribute("target").getValue());
				if (pair != mVC.end())
				{
					auto VC = pair->second;

					VC->setMin(child.getAttribute("min").getUintValue());
					VC->setMax(child.getAttribute("max").getUintValue());
					VC->set(name, child.getAttribute("value").getUintValue());

					mParams.add(*VC);
					mProgramGUI.add(*VC);

					auto midi = child.getChild("midi");
					if (midi)
					{
						int lu = (midi.getAttribute("channel").getIntValue() << 8) | midi.getAttribute("control").getIntValue();
						mMidiMap[lu] = VC;

						// send midi out
						paramChanged(*VC);
					}
				}
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

	mMenu.minimize();
	mProgramGUI.setRunning(false);
	mState = kStateProgramStopped;
}

void ofApp::closeProgram()
{
	if (mState == kStateProgramStopped || mState == kStateProgramRunning)
	{
		mProgramMutex.lock();
		
		if (mMidiOut.isOpen() && !mMidiMap.empty())
		{
			for (auto pair : mMidiMap)
			{
				int channel = pair.first >> 8;
				int control = pair.first & 255;
				mMidiOut.sendControlChange(channel, control, 0);
			}
		}

		delete mProgram;
		mProgram = nullptr;
		// clear the gui, this will delete all UI, including our mVars
		mProgramGUI.teardown();
		mKeyUI = nullptr;
		mVars.clear();
		mParams.clear();
		mMidiMap.clear();
    
		mProgramMutex.unlock();

		// have to reset this otherwise our menu options move when hovered over
		ofxBaseGui::setDefaultTextPadding(10);
		mMenu.maximize();
		mState = kStateMenu;
	}
}

//--------------------------------------------------------------
void ofApp::toggleProgramState()
{
	mProgramMutex.lock();
	if (mProgram != nullptr)
	{
		switch (mState)
		{
			case kStateProgramRunning: 
			{
				mState = kStateProgramStopped;
				mProgramGUI.setRunning(false);
			}
			break;
			
			case kStateProgramStopped:
			{
				mTick = 0;
				mState = kStateProgramRunning;
				mProgramGUI.setRunning(true);
			}
			break;
		}
	}
	mProgramMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::update()
{
	const float dt = 1.0f / ofGetFrameRate();

	if (mState == kStateProgramStopped || mState == kStateProgramRunning)
	{
		mProgramGUI.update(dt);
	}
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

	if (mState == kStateProgramRunning || mState == kStateProgramStopped)
	{
		if (mState == kStateProgramRunning)
		{
			// copy the contents of the audio thread buffer into our render thread buffer
			// and calculate which frame we need to start reading from.
			mOutputMutex.lock();
			mOutput.copyTo(mOutputRender);
			const size_t outputBegin = mOutputRead % frames;
			mOutputMutex.unlock();

			// render the buffer of samples
			for (size_t i = 0; i < frames; ++i)
			{
				int x = w * (i%cols);
				int y = h * (i / cols);
				size_t f = (outputBegin + i) % frames;
				ofSetColor((mOutputRender.getSample(f, 0) + 1)*127.5f);
				ofDrawRectangle(x, y, w, h);
				x += hw;
				ofSetColor((mOutputRender.getSample(f, 1) + 1)*127.5f);
				ofDrawRectangle(x, y, w, h);
			}
		}

		// draw the gui on top of it
		mProgramGUI.draw();
	}
	else
	{
		mMenu.draw();
	}
}

//--------------------------------------------------------------
void ofApp::exit()
{
	closeProgram();
	ofSoundStreamClose();
	mMidiIn.closePort();
	mMidiOut.closePort();
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs & args)
{
	if (mState == kStateProgramRunning || mState == kStateProgramStopped)
	{
		int key = args.key;

		// 24 is the 'x' key when control is held down
		if (key == OF_KEY_ESC || (args.hasModifier(OF_KEY_CONTROL) && key == 24))
		{
			closeProgram();
		}
		else if (key == OF_KEY_RETURN)
		{
			toggleProgramState();
		}
		else if (key == '+' && mProgramGUI.isMinimized())
		{
			mProgramGUI.maximize();
		}
		else if(key == '-' && !mProgramGUI.isMinimized())
		{
			mProgramGUI.minimize();
		}
		else if ( ((key >= OF_KEY_LEFT && key <= OF_KEY_DOWN) || (key >= '0' && key <= '9')) && mKeyUI != nullptr )
		{
			ofParameter<Program::Value>& param = mKeyUI->getParameter().cast<Program::Value>();
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
		else if (key >= 'a' && key <= 'z')
		{
			std::string name(1, key);
			mKeyUI = mProgramGUI.getControl(name);
		}
	}
	else if (args.key == OF_KEY_ESC)
	{
		ofExit();
	}
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
	if (mState == kStateProgramRunning || mState == kStateProgramStopped)
	{
		mProgramGUI.setSize(w, h);
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& message)
{
	if (message.status == MIDI_CONTROL_CHANGE)
	{
		int lu = (message.channel << 8) | message.control;
		auto iter = mMidiMap.find(lu);
		if (iter != mMidiMap.end())
		{
			auto param = iter->second;
			Program::Value val = ofMap(message.value, 0, 127, param->getMin(), param->getMax());
			param->set(val);
		}
	}
}

//--------------------------------------------------------------
void ofApp::paramChanged(ofAbstractParameter& param)
{
	if (mMidiOut.isOpen() && !mMidiMap.empty())
	{
		for (auto pair : mMidiMap)
		{
			auto v = pair.second;
			if (v->getName() == param.getName())
			{
				int channel = pair.first >> 8;
				int control = pair.first & 255;
				int value = ofMap(v->get(), v->getMin(), v->getMax(), 0, 127);
				mMidiOut.sendControlChange(channel, control, value);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::programSelected(int& which)
{
	if (mState == kStateMenu)
	{
		if (which >= 0 && which < mProgramList.size())
		{
			loadProgram(mProgramList[which]);
		}
	}
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& output)
{
	const size_t bufferSize = output.size();
	const size_t nChannels = output.getNumChannels();
	
	mProgramMutex.lock();
	if (mProgram != nullptr && mState == kStateProgramRunning)
	{
		const Program::Value range = (Program::Value)1 << mBitDepth;
		const float mdenom = mSoundSettings.sampleRate / 1000.0f;
		const float qdenom = (mSoundSettings.sampleRate / (mTempo / 60.0f)) / 128.0f;

		mProgram->Set('w', range);
		mProgram->Set('~', (Program::Value)mSoundSettings.sampleRate);
    
		for(size_t i = 0; i < mVars.size(); ++i)
		{
			mVars[i]->lock();
		}

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
    
		for(size_t i = 0; i < mVars.size(); ++i)
		{
			mVars[i]->unlock();
		}
	}
	mProgramMutex.unlock();

	mOutputMutex.lock();
	size_t writeBegin = (mTick*nChannels - bufferSize) % mOutput.size();
	const size_t outSize = mOutput.size();
	for (size_t f = 0; f < bufferSize; f += nChannels)
	{
		size_t i = (writeBegin + f) % outSize;
		mOutput[i] = output[f];
		mOutput[i + 1] = output[f + 1];
	}
	mOutputRead += bufferSize/nChannels;
	mOutputMutex.unlock();
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
