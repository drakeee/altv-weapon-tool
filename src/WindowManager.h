#pragma once

#include <string>
#include <map>
#include <functional>

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

	//typedef void(*ShortcutCallback)();
	typedef std::function<void()> ShortcutCallback;
	void AddShortcut(std::initializer_list<int> shortcut, ShortcutCallback callback);

	inline void CloseWindow() { this->m_ShouldQuit = true; }

	SDL_Texture* LoadTexture(std::string path);

	SDL_Window* m_Window = nullptr;
	SDL_Renderer* m_Renderer = nullptr;

	ImFont* m_InterFont = nullptr;
	ImFont* m_InterFontBold = nullptr;
	ImFont* m_InterFontExtraBold = nullptr;
	ImFont* m_AwesomeFont = nullptr;

	bool m_ShouldQuit = false;
	bool m_FullScreen = false;

private:
	void SetStyle();

	template<class T>
	inline void AddLayerComponent(T* layer)
	{
		this->m_LayerMap[typeid(T).name()] = layer;
	}

	template<class T>
	T* GetLayerComponent()
	{
		std::map<std::string, WindowLayer*>::iterator it = this->m_LayerMap.find(typeid(T).name());
		if (it != this->m_LayerMap.end())
			return static_cast<T*>(it->second);

		return nullptr;
	}

	void RenderLayers();
	void HandleKeyboard();

	int EventFilter(void* userdata, SDL_Event* event);
	SDL_HitTestResult HitTestCallback(SDL_Window* window, const SDL_Point* area, void* data);

	std::map<std::string, WindowLayer*> m_LayerMap;

	std::unordered_map<SDL_Scancode, bool> m_KeyPressed;
	std::map<std::vector<int>, ShortcutCallback> m_ShortcutMap;
};