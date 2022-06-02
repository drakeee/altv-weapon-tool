#pragma once

#include <string>

class WindowManager;
class WeaponTool
{
public:
	WeaponTool(std::string title = "Weapon Tool");
	~WeaponTool();

	void Run();

private:
	WindowManager* m_WindowManager;
};