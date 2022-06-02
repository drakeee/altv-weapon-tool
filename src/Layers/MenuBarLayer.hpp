#pragma once

#include <WindowLayer.h>

#include <filesystem>
#ifdef _WIN32
	#define NOMINMAX
	#include <Windows.h>
#endif

class MenuBarLayer : public WindowLayer
{
public:
	const std::string m_LogoPath = "./images/logo.png";
	SDL_Texture* m_LogoTexture = nullptr;

	uint32_t m_MenuHeight = 0;
	uint32_t m_MenuWidth = 0;

	//char m_WindowTitle[32];

	void Attach(WindowManager* manager) override
	{
		//sprintf_s(this->m_WindowTitle, sizeof(this->m_WindowTitle), "Weapon Tool");

		this->m_LogoTexture = manager->LoadTexture(this->m_LogoPath);

		SDL_Surface* tempSurface = IMG_Load(this->m_LogoPath.c_str());
		SDL_SetWindowIcon(manager->m_Window, tempSurface);
		SDL_FreeSurface(tempSurface);

		manager->AddShortcut({ SDL_SCANCODE_LCTRL, SDL_SCANCODE_O }, [&]()
			{
				printf("S: %s\n", this->OpenFile().c_str());
			});

		manager->AddShortcut({ SDL_SCANCODE_LCTRL, SDL_SCANCODE_X }, [=]()
			{
				manager->m_ShouldQuit = true;
			});
	}

	void PreRender(WindowManager* manager) override
	{

	}

	void Render(WindowManager* manager) override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::SetNextWindowBgAlpha(0.0f);
		if (ImGui::BeginMainMenuBar())
		{
			uint32_t menuBegin = ImGui::GetContentRegionAvail().x;
			this->m_MenuHeight = ImGui::GetWindowSize().y;

			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

				static const int logoSize = 32;
				ImGui::SetCursorPosY(this->m_MenuHeight / 2 - logoSize / 2);
				if (ImGui::ImageButton(this->m_LogoTexture, ImVec2(logoSize, logoSize)))
				{
					printf("alt:V\n");
					std::string myUrl("https://altv.mp");
					system(std::string("start " + myUrl).c_str());
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
				ImGui::SetCursorPosY(0);
			}

			ImGui::Spacing();
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{
					this->OpenFile();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Close", "Ctrl+X"))
				{
					manager->CloseWindow();
				}

				ImGui::EndMenu();
			}
			this->m_MenuWidth = (menuBegin - ImGui::GetContentRegionAvail().x);

			{
				ImGui::PushFont(manager->m_InterFontBold);
				float win_width = ImGui::GetWindowSize().x;
				float text_width = ImGui::CalcTextSize(SDL_GetWindowTitle(manager->GetWindow())).x;

				ImGui::SetCursorPosX(win_width / 2 - text_width / 2 - 10.0);
				ImGui::Text(SDL_GetWindowTitle(manager->GetWindow()));
				ImGui::PopFont();
			}

			{
				const static float buttonSize = /*32.0f*/ this->m_MenuHeight;
				float win_width = ImGui::GetWindowSize().x;

				ImGui::PushStyleColor(ImGuiCol_Button, IMGUI_COLOR(0x00, 0x87, 0x36, 0x00));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IMGUI_COLOR(0x04, 0x78, 0x32, 0xFF));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF));

				ImGui::PushFont(manager->m_AwesomeFont);

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
				ImGui::SetCursorPosX(win_width - (buttonSize * 3));
				if (ImGui::Button(u8"\uf2d1", ImVec2(buttonSize, buttonSize)))
				{
					SDL_MinimizeWindow(manager->m_Window);
				}

				ImGui::SetCursorPosX(win_width - (buttonSize * 2));
				if (ImGui::Button(manager->m_FullScreen ? u8"\uf2d2" : u8"\uf2d0", ImVec2(buttonSize, buttonSize)))
				{
					SDL_SetWindowFullscreen(manager->m_Window, !manager->m_FullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

					manager->m_FullScreen = !manager->m_FullScreen;
				}

				ImGui::SetCursorPosX(win_width - buttonSize);

				if (ImGui::Button(u8"\uf00d", ImVec2(buttonSize, buttonSize)))
				{
					manager->CloseWindow();
				}

				ImGui::PopStyleVar();
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

			}

			ImGui::EndMainMenuBar();
		}

		ImGui::PopStyleVar(2);
	}

private:
	std::string OpenFile()
	{
		OPENFILENAME ofn;
		char fileName[MAX_PATH] = "";
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "Meta Files (*.meta)\0*.meta\0";
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "";
		ofn.lpstrInitialDir = strdup(std::filesystem::current_path().string().c_str());

		std::string fileNameStr("");

		if (GetOpenFileName(&ofn))
			fileNameStr = fileName;

		return fileNameStr;
	}
};