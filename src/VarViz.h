#pragma once

#include "Program.h"
#include "ofxBaseGui.h"

enum VarVizType
{
	kVizTypeValue,
	kVizTypeWave,
	kVizTypeBars,
	kVizTypeScatter,
	kVizTypeBlocks,
};

class VarViz : public ofxBaseGui
{
public:
	VarViz( const std::string& guiName, Program::Value var, const VarVizType vizType = kVizTypeWave, const size_t bufferSize = 1024, const size_t sampleRate = 1, const Program::Value range = 0)
	{
		setup(guiName, var, vizType, bufferSize, sampleRate, range);
	}

	~VarViz();

	VarViz* setup(const std::string& guiName, Program::Value var, const VarVizType vizType = kVizTypeWave, const size_t bufferSize = 1024, const size_t sampleRate = 1, const Program::Value range = 0);

	Program::Value getVar() const { return mVar; }
	void setColumns(size_t cols) { mColumns = cols; }

	void push(Program::Value value);


	virtual ofAbstractParameter & getParameter() override;


	virtual bool mouseMoved(ofMouseEventArgs & args) override { return false; }
	virtual bool mousePressed(ofMouseEventArgs & args) override { return false; }
	virtual bool mouseDragged(ofMouseEventArgs & args) override { return false; }
	virtual bool mouseReleased(ofMouseEventArgs & args) override { return false; }
	virtual bool mouseScrolled(ofMouseEventArgs & args) override { return false; }

protected:
	void render();
	void generateDraw();

	ofPath mBack;
	ofPath mViz;
	ofVboMesh mLabel;


	virtual bool setValue(float mx, float my, bool bCheckBounds) override { return false; }

private:

	ofParameter<Program::Value> mVar;
	VarVizType mVizType;
	size_t mSize;
	size_t mSampleRate;
	size_t mColumns;
	Program::Value mRange;
	Program::Value mMax;
	std::vector<Program::Value> mBuffer;
	size_t mHead;
	size_t mCount;
};

