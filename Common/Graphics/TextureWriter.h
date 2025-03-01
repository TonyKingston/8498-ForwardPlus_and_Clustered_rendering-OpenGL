/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include <string>

namespace NCL {
	class Image;

	namespace Rendering {
		class TextureBase;
	}

	class TextureWriter
	{
	public:
		static void WritePNG(const std::string& filename, char* data, int width, int height, int channels);
		static bool SaveImage(const std::string& filename, const Image& image, int quality = 100);

		/// <summary>
		/// Save texture data to a file. Saves to Assets::OUTPUTDIR by default
		/// </summary>
		/// <param name="filename">Filename to save to with extension.</param>
		/// <param name="texture">Texture with raw data to save</param>
		/// <param name="quality">Determines output quality of JPEG files, ignored otherwise</param>
		static bool SaveTexture(const std::string& filename, const Rendering::TextureBase* texture, int quality = 100);
	};
}

