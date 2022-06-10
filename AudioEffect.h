#pragma once
#include "Sound.h"
#include <QtWidgets/qapplication.h>
#include <windows.h>
class AudioEffect
{
public:
	static void Warning() {
		QApplication::beep();
	}
	static void TestActive() {
		Beep(note_do, 400);
	}
};

