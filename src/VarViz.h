#pragma once

#include "Program.h"

class VarViz
{
public:
	VarViz(const char varName, const size_t bufferSize, const size_t sampleRate, float x, float y, float w, float h);
	~VarViz();

	const char Var;

	void push(Program::Value value);

	void draw();

private:

	const size_t mSize;
	const size_t mSampleRate;
	const float mX, mY, mW, mH;
	Program::Value* mBuffer;
	size_t mHead;
	size_t mCount;
};

