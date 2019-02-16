[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)

# X3J11 - formulae
five bytebeat compositions generated in real-time by your computer

The program shown for each composition is written in a C-like language that is executed to generate the music. These compositions do not have endings, per se, and will run indefinitely.

Each program includes an expandable overlay that visualizes some of the variables in the program. Many of these include sliders that can be used to change the program while it plays.

**Controls**

In the menu, use the arrow keys to select a program and press the enter key to load it,  or simply click on one of the program names with the mouse.

To listen to a loaded program, press the enter key or click on "> run".

To stop a running program, press the enter key or click on "> stop".

To show the overlay for a program, click the + icon or press the + key. Clicking it again or pressing the - key will hide it.

Sliders in the overlay can be changed by clicking and dragging with the mouse or by pressing the keyboard key that corresponds to the name on the left side of a slider and using the arrow keys to change the value.

To close a program and return to the main menu, click the x icon or press ctrl+x on the keyboard.

To exit the app, press the Esc key (or Cmd+Q on Mac) from the main menu.

If you happen to own a Midi Fighter Twister, you can load `twister.mfs` onto it and use it to control the four compositions that have sliders. It is configured to fit the controls for all four compositions in the first bank. Knobs are color-coded and arranged similarly to the onscreen UI.

**Building**

This project builds against [openFrameworks v0.10.1](https://openframeworks.cc/download/) and requires the [ofxMidi addon](https://github.com/danomatika/ofxMidi). Development was completed in Visual Studio 2017 and XCode 10.1.

**Tweaking**

This app is almost entirely data-driven by the `settings.xml` file in `bin\data`, should you want to experiment with layout, program content, or midi configuration.
