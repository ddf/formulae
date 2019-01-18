#pragma once

#include "ofxGuiGroup.h"

class ProgramGUI : public ofxGuiGroup
{
public:
	ProgramGUI();
	ProgramGUI(const ofParameterGroup & parameters, const std::string& _filename = "settings.xml", float x = 10, float y = 10);
	~ProgramGUI();

	void setup(const std::string& collectionName /* = "" */, const std::string& filename /* = "settings.xml" */, float x /* = 10 */, float y /* = 10 */);
	void teardown();

	// overridden to change the auto-layout behavior of ofxGuiGroup
	virtual void sizeChangedCB() override;

	ofEvent<void> closePressedE;

protected:
	bool setValue(float mx, float my, bool bCheck);
	void generateDraw();
	
};

