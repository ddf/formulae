#pragma once
#include "ofxButton.h"

class Button : public ofxButton
{
public:
	Button();
	Button(ofParameter<int> _bVal, float width = defaultWidth, float height = defaultHeight);
	~Button();

	Button* setup(ofParameter<int> _bVal, float width = defaultWidth, float height = defaultHeight);
	Button* setup(const std::string& toggleName, int bVal, float width = defaultWidth, float height = defaultHeight);

	virtual bool mouseMoved(ofMouseEventArgs & args) override;

	void setFocused(bool focus);
	bool isFocused() const { return focused; }
	int getID() const { return id; }

	ofEvent<int> clickedE;
	ofEvent<int> hoveredE;

protected:
	virtual void generateDraw() override;

	void onClick();

	int  id;
	bool hover;
	bool focused;
};

