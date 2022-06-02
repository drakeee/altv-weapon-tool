#include <iostream>

#include <WindowManager.h>

#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <Layers/BackgroundLayer.hpp>
#include <Layers/MenuBarLayer.hpp>
#include <Layers/MainTabLayer.hpp>

WindowManager::WindowManager(const char* title, int width, int height)
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL. Error: %s\n", SDL_GetError());
		return;
	}

	this->m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!this->m_Window)
	{
		printf("Unable to create window. Error: %s\n", SDL_GetError());
		return;
	}

	this->m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!m_Renderer)
	{
		printf("Unable to create renderer. Error: %s\n", SDL_GetError());
		return;
	}

	SDL_SetWindowHitTest(this->m_Window, [](SDL_Window* window, const SDL_Point* area, void* data) -> SDL_HitTestResult
		{
			WindowManager* manager = reinterpret_cast<WindowManager*>(data);
			return manager->HitTestCallback(window, area, data);
		}, this);

	SDL_SetWindowResizable(this->m_Window, SDL_TRUE);
	SDL_AddEventWatch([](void* userdata, SDL_Event* event) -> int
		{
			WindowManager* manager = reinterpret_cast<WindowManager*>(userdata);
			return manager->EventFilter(manager->GetWindow(), event);
		}, this);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontDefault();

	this->m_InterFont = io.Fonts->AddFontFromFileTTF("fonts\\Inter-Medium.otf", 18);
	this->m_InterFontBold = io.Fonts->AddFontFromFileTTF("fonts\\Inter-Bold.otf", 18);
	this->m_InterFontExtraBold = io.Fonts->AddFontFromFileTTF("fonts\\Inter-ExtraBold.otf", 18);

	ImFontConfig awesomeConfig;
	awesomeConfig.MergeMode = false;
	//awesomeConfig.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced

	static const ImWchar icon_ranges[] = { 0xf000, 0xf3ff, 0 };
	this->m_AwesomeFont = io.Fonts->AddFontFromFileTTF("fonts\\fa-solid-900.ttf", 12, &awesomeConfig, icon_ranges);
	//io.Fonts->Build();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	this->SetStyle();

	ImGui_ImplSDL2_InitForSDLRenderer(this->m_Window, this->m_Renderer);
	ImGui_ImplSDLRenderer_Init(this->m_Renderer);

	this->AddLayerComponent(new BackgroundLayer);
	this->AddLayerComponent(new MenuBarLayer);
	this->AddLayerComponent(new MainTabLayer);

	for (auto& layer : this->m_LayerMap)
		layer.second->Attach(this);
}

WindowManager::~WindowManager()
{

}

void WindowManager::Render()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
		case SDL_KEYUP:
		case SDL_KEYDOWN:
		{
			if (!event.key.repeat)
			{
				if (event.key.type == SDL_KEYDOWN)
				{
					this->m_KeyPressed[event.key.keysym.scancode] = (event.key.type == SDL_KEYDOWN);
					this->HandleKeyboard();
				} else
					this->m_KeyPressed.erase(event.key.keysym.scancode);
			}

			break;
		}
		case SDL_QUIT:
		{
			this->CloseWindow();
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				this->CloseWindow();

			break;
		}
		}

		ImGui_ImplSDL2_ProcessEvent(&event);
	}

	if (event.type != SDL_WINDOWEVENT)
	{
		this->RenderLayers();
	}
}

void WindowManager::RenderLayers()
{
	SDL_SetRenderDrawColor(this->m_Renderer, 120, 120, 120, 0);
	SDL_RenderClear(this->m_Renderer);

	for (auto& layer : this->m_LayerMap)
		layer.second->PreRender(this);

	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	//this->m_InterFont->Scale = (16.0f/this->m_InterFont->FontSize);
	ImGui::PushFont(this->m_InterFont);

	for (auto& layer : this->m_LayerMap)
		layer.second->Render(this);

	ImGui::PopFont();
	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(this->m_Renderer);
}

void WindowManager::HandleKeyboard()
{
	//Transform map keys into vectors of keys
	std::vector<int> keys;
	std::transform(this->m_KeyPressed.begin(), this->m_KeyPressed.end(), std::back_inserter(keys), [](const std::pair<SDL_Scancode, bool>& p) -> SDL_Scancode
	{
		return p.first;
	});

	//Check if we have a shortcut with given keys
	std::map<std::vector<int>, ShortcutCallback>::iterator it = this->m_ShortcutMap.find(keys);
	if (it != this->m_ShortcutMap.end())
	{
		//Execute shortcut's callback
		it->second();
	}
}

SDL_Texture* WindowManager::LoadTexture(std::string path)
{
	//Check if we have a renderer
	if (!m_Renderer)
		return nullptr;

	//Load image from given path
	SDL_Surface* imageSurface = IMG_Load(path.c_str());
	if (!imageSurface)
	{
		printf("Unable to load image. Error: %s\n", SDL_GetError());
		return nullptr;
	}

	//Convert surface into texture
	SDL_Texture* returnTexture = SDL_CreateTextureFromSurface(this->m_Renderer, imageSurface);
	SDL_FreeSurface(imageSurface);

	//Return texture
	return returnTexture;
}

void WindowManager::AddShortcut(std::initializer_list<int> shortcut, ShortcutCallback callback)
{
	this->m_ShortcutMap[shortcut] = callback;
}

void WindowManager::SetStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 5.0f;
	style.FrameRounding = 5.0f;
	style.TabRounding = 5.0f;
	style.PopupRounding = 5.0f;
	/*style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;*/

	//style.Colors[ImGuiCol_MenuBarBg] = IMGUI_COLOR(0x29, 0x29, 0x29, 150);
	style.Colors[ImGuiCol_MenuBarBg] = IMGUI_COLOR(0, 0, 0, 0);

	style.Colors[ImGuiCol_Tab] = IMGUI_COLOR(0x00, 0x87, 0x36, 0xFF);
	style.Colors[ImGuiCol_TabHovered] = IMGUI_COLOR(0x04, 0x78, 0x32, 0xFF);
	//style.Colors[ImGuiCol_TabActive] = IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF);
	style.Colors[ImGuiCol_TabActive] = IMGUI_COLOR(0x00, 0x87, 0x36, 0xFF);

	style.Colors[ImGuiCol_TableRowBg] = IMGUI_COLOR(255, 255, 255, 0);
	style.Colors[ImGuiCol_TableRowBgAlt] = IMGUI_COLOR(255, 255, 255, 5);

	style.Colors[ImGuiCol_FrameBg] = IMGUI_COLOR(30, 30, 30, 0);
	style.Colors[ImGuiCol_FrameBgHovered] = IMGUI_COLOR(0, 0, 0, 255);

	style.Colors[ImGuiCol_Header] = IMGUI_COLOR(0x00, 0x87, 0x36, 0xFF);
	style.Colors[ImGuiCol_HeaderHovered] = IMGUI_COLOR(0x04, 0x78, 0x32, 0xFF);
	style.Colors[ImGuiCol_HeaderActive] = IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF);

	style.Colors[ImGuiCol_ScrollbarBg] = IMGUI_COLOR(0, 0, 0, 0);
}

/*void WindowManager::AddLayer(std::string layerName, WindowLayer* layer)
{
	this->m_LayerMap[layerName] = layer;
}

WindowLayer* WindowManager::GetLayer(std::string layerName)
{
	std::map<std::string, WindowLayer*>::iterator it = this->m_LayerMap.find(layerName);
	if (it != this->m_LayerMap.end())
		return it->second;

	return nullptr;
}*/

int WindowManager::EventFilter(void* userdata, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT)
		if (event->window.event == SDL_WINDOWEVENT_EXPOSED)
			this->RenderLayers();

	return 0;
}

SDL_HitTestResult WindowManager::HitTestCallback(SDL_Window* window, const SDL_Point* area, void* data)
{
	static const int resizePadding = 7;
	static const int cornerSize = 16;

	int windowWidth, windowHeight;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);

	MenuBarLayer* menuBar = this->GetLayerComponent<MenuBarLayer>();
	if (menuBar == nullptr)
		return SDL_HitTestResult::SDL_HITTEST_NORMAL;

	//Check if we are dragging the menu bar
	if (
		(area->x > menuBar->m_MenuWidth) &&
		(area->x < (windowWidth - (3 * menuBar->m_MenuHeight))) &&
		(area->y <= menuBar->m_MenuHeight)
		)
	{
		return SDL_HitTestResult::SDL_HITTEST_DRAGGABLE;
	}

	//Check borders of the window for resizing
	if (area->y > menuBar->m_MenuHeight)
	{
		if (area->x < resizePadding && (area->y < (windowHeight - cornerSize)))
		{
			return SDL_HitTestResult::SDL_HITTEST_RESIZE_LEFT;
		}

		if (area->x > (windowWidth - resizePadding) && (area->y < (windowHeight - cornerSize)))
		{
			return SDL_HitTestResult::SDL_HITTEST_RESIZE_RIGHT;
		}

		if (area->y > (windowHeight - resizePadding) && (area->x < (windowWidth - cornerSize)) && (area->x > cornerSize))
		{
			return SDL_HitTestResult::SDL_HITTEST_RESIZE_BOTTOM;
		}

		if (area->y > (windowHeight - cornerSize) && (area->x < cornerSize))
		{
			return SDL_HitTestResult::SDL_HITTEST_RESIZE_BOTTOMLEFT;
		}

		if (area->y > (windowHeight - cornerSize) && (area->x > (windowWidth - cornerSize)))
		{
			return SDL_HitTestResult::SDL_HITTEST_RESIZE_BOTTOMRIGHT;
		}
	}

	return SDL_HitTestResult::SDL_HITTEST_NORMAL;
}