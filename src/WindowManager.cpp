#include <iostream>

#include <WindowManager.h>

#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <Layers/BackgroundLayer.hpp>
#include <Layers/MenuBarLayer.hpp>

WindowManager::WindowManager(const char* title, int width, int height)
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL. Error: %s\n", SDL_GetError());
		return;
	}

	this->m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
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

	this->m_InterFont = io.Fonts->AddFontFromFileTTF("fonts\\Inter-Medium.otf", 16);

	ImFontConfig config;
	config.MergeMode = false;
	//config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced

	static const ImWchar icon_ranges[] = { 0xf000, 0xf3ff, 0 };
	this->m_AwesomeFont = io.Fonts->AddFontFromFileTTF("fonts\\fa-solid-900.ttf", 12, &config, icon_ranges);
	//io.Fonts->Build();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	this->SetStyle();

	ImGui_ImplSDL2_InitForSDLRenderer(this->m_Window, this->m_Renderer);
	ImGui_ImplSDLRenderer_Init(this->m_Renderer);

	this->AddLayer("Background", new BackgroundLayer);
	this->AddLayer("MenuBar", new MenuBarLayer);

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
		case SDL_QUIT:
		{
			this->m_ShouldQuit = true;
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				this->m_ShouldQuit = true;

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

	for (auto& layer : this->m_LayerMap)
		layer.second->Render(this);

	//RenderMenu();

	/*ImGui::Begin("Some Window");
	ImGui::Text("Some text to show");
	ImGui::End();*/

	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(this->m_Renderer);
}

SDL_Texture* WindowManager::LoadTexture(std::string path)
{
	if (!m_Renderer)
		return nullptr;

	SDL_Surface* imageSurface = IMG_Load(path.c_str());
	if (!imageSurface)
	{
		printf("Unable to load image. Error: %s\n", SDL_GetError());
		return nullptr;
	}

	SDL_Texture* returnTexture = SDL_CreateTextureFromSurface(this->m_Renderer, imageSurface);
	SDL_FreeSurface(imageSurface);

	return returnTexture;
}

void WindowManager::SetStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_MenuBarBg] = IMGUI_COLOR(0x29, 0x29, 0x29, 150);
	style.Colors[ImGuiCol_Tab] = IMGUI_COLOR(60, 102, 145, 150);
	style.Colors[ImGuiCol_TabActive] = IMGUI_COLOR(60, 102, 145, 255);
}

void WindowManager::AddLayer(std::string layerName, WindowLayer* layer)
{
	this->m_LayerMap[layerName] = layer;
}

WindowLayer* WindowManager::GetLayer(std::string layerName)
{
	std::map<std::string, WindowLayer*>::iterator it = this->m_LayerMap.find(layerName);
	if (it != this->m_LayerMap.end())
		return it->second;

	return nullptr;
}

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

	MenuBarLayer* menuBar = this->GetLayer<MenuBarLayer>("MenuBar");

	if (
		(area->x > menuBar->m_MenuWidth) &&
		(area->x < (windowWidth - (3 * menuBar->m_MenuHeight))) &&
		(area->y <= menuBar->m_MenuHeight)
		)
	{
		return SDL_HitTestResult::SDL_HITTEST_DRAGGABLE;
	}

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