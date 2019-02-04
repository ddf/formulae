#pragma once
#include "ofxGuiGroup.h"

#include "Button.h"

class MenuGUI :	public ofxGuiGroup
{
public:
	MenuGUI();
	~MenuGUI();

	MenuGUI * setup(const std::string& collectionName = "", const std::string& filename = "menu.xml", float x = 10, float y = 10);

	void add(std::string & buttonName);

	ofEvent<int> buttonClickedE;

protected:
	bool setValue(float mx, float my, bool bCheck) override;
	void generateDraw() override;

	void onButtonClick(int& id);
};

