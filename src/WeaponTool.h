#pragma once

class WindowManager;
class WeaponTool
{
public:
	WeaponTool();
	~WeaponTool();

	void Run();

private:
	WindowManager* m_WindowManager;
};