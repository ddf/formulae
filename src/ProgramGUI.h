#pragma once

#include "ofxGuiGroup.h"

class ProgramGUI : public ofxGuiGroup
{
public:
	ProgramGUI();
	ProgramGUI(const ofParameterGroup & parameters, const std::string& _filename = "settings.xml", float x = 10, float y = 10);
	~ProgramGUI();

	// overridden to change the auto-layout behavior of ofxGuiGroup
	virtual void sizeChangedCB() override;
};

