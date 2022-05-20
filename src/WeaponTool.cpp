#include <WeaponTool.h>
#include <WindowManager.h>

WeaponTool::WeaponTool() :
	m_WindowManager(new WindowManager("alt:V - Weapon Tool"))
{

}

WeaponTool::~WeaponTool()
{

}

void WeaponTool::Run()
{
	while (!this->m_WindowManager->m_ShouldQuit)
	{
		this->m_WindowManager->Render();
	}
}