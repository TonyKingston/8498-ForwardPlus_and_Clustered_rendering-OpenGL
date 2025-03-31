/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "pch.h"
#include "TextureLoader.h"
#include <iostream>
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

	std::filesystem::path path(filename);
	std::string extension = path.extension().string();

	auto it = fileHandlers.find(extension);

	std::string realPath = path.is_absolute() ? filename : Assets::TEXTUREDIR + filename;

	auto [width, height, channels] = std::tuple{ 0, 0, 0 };
	if (it != fileHandlers.end()) {
		//There's a custom handler function for this, just use that
		return it->second(realPath, outImage, flags);
	}
	// Will call move assignment which frees memory if outImage already contains data for some reason.
	outImage = Image(realPath.c_str());

	return outImage.IsValid();
}

void TextureLoader::RegisterTextureLoadFunction(TextureLoadFunction f, const std::string&fileExtension) {
	fileHandlers.insert(std::make_pair(fileExtension, f));
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