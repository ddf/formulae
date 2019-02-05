#pragma once

#include "ofxGuiGroup.h"

class ProgramGUI : public ofxGuiGroup
{
public:
	ProgramGUI();
	ProgramGUI(const ofParameterGroup & parameters, const std::string& _filename = "settings.xml", float x = 10, float y = 10);
	~ProgramGUI();

	void setup(const std::string& collectionName /* = "" */, const std::string& code, const std::string& filename /* = "settings.xml" */, float x /* = 10 */, float y /* = 10 */);
	void teardown();

	void update(const float dt);

	virtual void setSize(float x, float y) override;
	virtual void setShape(ofRectangle r) override;
	virtual void setShape(float x, float y, float w, float h) override;

	// overridden to change the auto-layout behavior of ofxGuiGroup
	virtual void sizeChangedCB() override;

	void setRunning(bool state);

	ofEvent<void> closePressedE;
	ofEvent<void> transportPressedE;

protected:
	bool setValue(float mx, float my, bool bCheck) override;
	void generateDraw() override;

	virtual void onMaximize() override;

	// program split into lines so we can animate the print out
	std::vector<std::string> mProgramCode;
	float mProgramAnim;
	int mProgramLines;
	ofRectangle mRunRect;
	bool mbRunning;
	float mW, mH;
};

