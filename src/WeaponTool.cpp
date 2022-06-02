#include <WeaponTool.h>
#include <WindowManager.h>

WeaponTool::WeaponTool(std::string title) :
	m_WindowManager(new WindowManager(title.c_str()))
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