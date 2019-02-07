#include "ofMain.h"
#include "ofApp.h"

//========================================================================
// for why Windows is different, see: https://forum.openframeworks.cc/t/how-to-get-rid-of-console-and-set-proper-app-icon/24227

#ifdef WIN32
#include "../resource.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main( )
#endif
{
	ofSetupOpenGL(1920,1080,OF_WINDOW);			// <-------- setup the GL context

#ifdef WIN32
	HWND hwnd = ofGetWin32Window();
	HICON hMyIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MAIN_ICON));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMyIcon);
#endif

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}
