#pragma once

#include "Program.h"

enum VarVizType
{
	kVizTypeWave,
	kVizTypeBars,
	kVizTypeScatter,
	kVizTypeBlocks,
};

class VarViz
{
public:
	VarViz(const char varName, float x, float y, float w, float h, const VarVizType vizType, const size_t bufferSize, const size_t sampleRate = 1, const Program::Value range = 0);
	~VarViz();

	const char Var;

	void push(Program::Value value);

	void draw();

private:

	const VarVizType mVizType;
	const size_t mSize;
	const size_t mSampleRate;
	const Program::Value mRange;
	const float mX, mY, mW, mH;
	Program::Value* mBuffer;
	Program::Value mMax;
	size_t mHead;
	size_t mCount;
};

