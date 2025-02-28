/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <map>
#include <functional>
#include <string>
using std::map;

#include "TextureBase.h"

namespace NCL {

	class Image;
	// TODO: Might just replace with optional<Image> instead of returning bool
	typedef std::function<bool(const std::string& filename, Image& outImage, int& flags)> TextureLoadFunction;

	typedef std::function<Rendering::TextureBase*(const std::string& filename)> APILoadFunction;

	class TextureLoader	{
	public:
		/// <summary>
		/// Loads a texture from the texture directory with a given name
		/// </summary>
		/// <param name="filename">Name of texture to load</param>
		/// <param name="outImage">Output image. If the passed in Image already has data, said data will be freed</param>
		/// <param name="flags">Currently unused flags</param>
		/// <returns>True if the image could be loaded, false otherwise.</returns>
		static bool LoadTexture(const std::string& filename, Image& outImage, int&flags);


		/// <summary>
		/// Register a specialised function to handle textures will particular file extensions.
		/// This function will be used in LoadTexture instead of the default behaviour.
		/// </summary>
		/// <param name="f">Custom texture load functor</param>
		/// <param name="fileExtension">Extension to apply the function on e.g. "png"</param>
		static void RegisterTextureLoadFunction(TextureLoadFunction f, const std::string&fileExtension);

		static void RegisterAPILoadFunction(APILoadFunction f);

		/// <summary>
		/// Calls the registered API texture loading function (if it exists)
		/// </summary>
		/// <param name="filename"></param>
		/// <returns>Pointer to loaded texture</returns>
		static Rendering::TextureBase* LoadAPITexture(const std::string&filename);
	protected:

		static std::string GetFileExtension(const std::string& fileExtension);

		static std::map<std::string, TextureLoadFunction> fileHandlers;

		static APILoadFunction apiFunction;
	};
}

