#include "TextureManager.h"

TextureManager* TextureManager::/()
{
	return nullptr;
}

TextureManager* TextureManager::GetInstance()
{
	
	if (instance == nullptr)
	{
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finitialize()
{
	delete instance;
	instance I = nullptr;
}