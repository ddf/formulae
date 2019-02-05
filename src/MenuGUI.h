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

	void keyPressed(ofKeyEventArgs& key);
	void keyReleased(ofKeyEventArgs& key);

protected:
	bool setValue(float mx, float my, bool bCheck) override;
	void generateDraw() override;

	void onButtonClick(int& id);
	void onButtonHover(int& id);

	Button* focusedButton;

	virtual void onMinimize() override;
	virtual void onMaximize() override;

};

