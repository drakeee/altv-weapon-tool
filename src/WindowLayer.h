#pragma once

#include <WindowManager.h>

class WindowManager;
class WindowLayer
{
public:
	virtual void Attach(WindowManager*) = 0;
	virtual void PreRender(WindowManager*) = 0;
	virtual void Render(WindowManager*) = 0;
};