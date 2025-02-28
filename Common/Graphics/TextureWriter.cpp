/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "pch.h"
#include "TextureWriter.h"
#include "TextureBase.h"
#include "Core/Misc/Image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../stb/stb_image_write.h"
#include "Resources/Assets.h"
using namespace NCL;

void TextureWriter::WritePNG(const std::string& filename, char* data, int width, int height, int channels) {
	stbi_write_png(filename.c_str(), width, height, channels, data, width * channels);
}

bool NCL::TextureWriter::SaveImage(const std::string& filename, const Image& image, int quality /*=100*/) {
	namespace fs = std::filesystem;
	fs::path filePath = fs::path(filename);
	// If the user has just supplied the filename, output to the default directory
	filePath = filePath.has_parent_path() ? filePath : Assets::OUTPUTDIR / filePath;
	std::string extension = filePath.extension().string();

	static const std::unordered_map<std::string, std::function<int(const char*, int, int, int, const void*)>> writers = {
			{".png", [](const char* filename, int w, int h, int c, const void* d) { return stbi_write_png(filename, w, h, c, d, w * c); }},
			{".bmp", stbi_write_bmp},
			{".tga", stbi_write_tga},
			{".jpg", [quality](const char* filename, int w, int h, int c, const void* d) { return stbi_write_jpg(filename, w, h, c, d, quality); }}
	};

	auto it = writers.find(extension);
	if (it != writers.end()) {
		return it->second(filePath.string().c_str(), (int)image.width, (int)image.height, (int)image.channels, (void*)image.data);
	}
	
	LOG_ERROR("Could not find save function for file {} when trying to save image", filename);
}

bool NCL::TextureWriter::SaveTexture(const std::string& filename, const Rendering::TextureBase* texture, int quality /*100*/) {
	return SaveImage(filename, texture->GetRawTextureData(), quality);
}
