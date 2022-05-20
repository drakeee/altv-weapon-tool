#pragma once

#include <WindowLayer.h>
#include <iostream>

class BackgroundLayer : public WindowLayer
{
	const std::string m_BackgroundPath = "./images/background.png";
	SDL_Texture* m_BackgroundTexture = nullptr;

	void Attach(WindowManager* manager)
	{
		this->m_BackgroundTexture = manager->LoadTexture(this->m_BackgroundPath);
	}

	void PreRender(WindowManager* manager) override
	{
		const static int originalWidth = 1920;
		const static int originalHeight = 1080;

		int width, height;
		SDL_GetWindowSize(manager->m_Window, &width, &height);

		float ratioX = (float)width / (float)originalWidth;
		float ratioY = (float)height / (float)originalHeight;
		float ratio = std::max(ratioX, ratioY);

		int newWidth = (int)(originalWidth * ratio);
		int newHeight = (int)(originalHeight * ratio);

		SDL_Rect destination;
		destination.x = (width / 2) - (newWidth / 2);
		destination.y = (height / 2) - (newHeight / 2);
		destination.w = newWidth;
		destination.h = newHeight;

		SDL_RenderCopy(manager->m_Renderer, this->m_BackgroundTexture, NULL, &destination);
	}

	void Render(WindowManager* manager) override
	{
		
	}
};