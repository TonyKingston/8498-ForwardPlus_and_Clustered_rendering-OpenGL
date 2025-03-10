/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "pch.h"
#include "TextureLoader.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION

#include "./stb/stb_image.h"

#include "Resources/Assets.h"
#include "Core/Misc/Image.h"

#ifdef WIN32
using namespace std::filesystem;
#endif

using namespace NCL;
using namespace Rendering;

std::map<std::string, TextureLoadFunction> TextureLoader::fileHandlers;
APILoadFunction TextureLoader::apiFunction = nullptr;

bool TextureLoader::LoadTexture(const std::string& filename, Image& outImage, int& flags) {
	if (filename.empty()) {
		LOG_ERROR("Trying to load texture with empty filename");
		return false;
	}
	std::string extension = GetFileExtension(filename);

	auto it = fileHandlers.find(extension);

	std::string realPath = Assets::TEXTUREDIR + filename;

	auto [width, height, channels] = std::tuple{ 0, 0, 0 };
	if (it != fileHandlers.end()) {
		//There's a custom handler function for this, just use that
		return it->second(realPath, outImage, flags);
	}
	//By default, attempt to use stb image to get this texture
	stbi_uc *texData = stbi_load(realPath.c_str(), &width, &height, &channels, 4); //4 forces this to always be rgba!

	if (texData) {
		// Will call move assignment which frees memory.
		outImage = Image(texData, width, height, channels);
		return true;
	}

	return false;
}

void TextureLoader::RegisterTextureLoadFunction(TextureLoadFunction f, const std::string&fileExtension) {
	fileHandlers.insert(std::make_pair(fileExtension, f));
}

std::string TextureLoader::GetFileExtension(const std::string& fileExtension) {
#ifdef WIN32
	path p = path(fileExtension);

	path ext = p.extension();

	return ext.string();
#else
	return std::string();
#endif
}

void TextureLoader::RegisterAPILoadFunction(APILoadFunction f) {
	if (apiFunction) {
		LOG_INFO("{} replacing previously defined API function", __FUNCTION__);
	}
	apiFunction = f;
}

TextureBase* TextureLoader::LoadAPITexture(const std::string&filename) {
	if (apiFunction == nullptr) {
		LOG_ERROR("{} no API function has been defined", __FUNCTION__);
		return nullptr;
	}
	return apiFunction(filename);
}