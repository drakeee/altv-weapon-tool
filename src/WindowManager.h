#pragma once

#include <string>
#include <map>

#include <SDL.h>
#include <imgui.h>

#define IMGUI_COLOR(r,g,b,a) ImVec4(r/255.0f,g/255.0f, b/255.0f, a/255.0f)

class WindowLayer;
class WindowManager
{
public:
	WindowManager(const char* title, int width = 1280, int height = 720);
	~WindowManager();

	inline SDL_Window* GetWindow() { return this->m_Window; }
	inline SDL_Renderer* GetRenderer() { return this->m_Renderer; }

	void Render();

	SDL_Texture* LoadTexture(std::string path);

	SDL_Window* m_Window = nullptr;
	SDL_Renderer* m_Renderer = nullptr;

	ImFont* m_InterFont = nullptr;
	ImFont* m_AwesomeFont = nullptr;

	bool m_ShouldQuit = false;
	bool m_FullScreen = false;

private:
	void SetStyle();

	void AddLayer(std::string layerName, WindowLayer* layer);
	WindowLayer* GetLayer(std::string layerName);

	template<typename T>
	T* GetLayer(std::string layerName)
	{
		return static_cast<T*>(this->GetLayer(layerName));
	}

	void RenderLayers();

	int EventFilter(void* userdata, SDL_Event* event);
	SDL_HitTestResult HitTestCallback(SDL_Window* window, const SDL_Point* area, void* data);

	std::map<std::string, WindowLayer*> m_LayerMap;
};