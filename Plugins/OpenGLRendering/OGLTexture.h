/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Common/Graphics/TextureBase.h"
#include "Common/Math/Maths.h"
#include "Common/NCLAliases.h"
#include "glad\glad.h"
#include <string>

using namespace NCL::Maths;

// Whether Direct Storage Access should be used for texture creation
#ifdef GL_VERSION_4_5
#define USE_DSA 1
#else
#define USE_DSA 0
#endif

namespace NCL {

	class Image;
	namespace Rendering {

		// I've adpated some of https://github.com/JuanDiegoMontoya/Fwog/
		struct TextureConfig {
			ImageType imageType;
			ImageFormat format;
			IntVector3 extent;
			uint32_t mipLevels = 0;
			uint32_t arrayLayers = 0;
			uint32_t sampleCount = 1;

			bool operator==(const TextureConfig&) const noexcept = default;
		};

		constexpr GLint ImageTypeToOGL(ImageType imageType);
		constexpr GLint FormatToOGL(ImageFormat format);
		constexpr GLint PixelFormatToOGL(PixelFormat format);

		class OGLTexture : public TextureBase
		{
		public:
			//friend class OGLRenderer;
			// TODO: Should only have one ctor
			 OGLTexture(GLuint texToOwn);
			 OGLTexture(const TextureConfig& inConfig, std::string_view name = "");

			 OGLTexture(OGLTexture&& other) noexcept : texID(std::exchange(other.texID, 0)), config(other.config) {}
			 OGLTexture& operator=(OGLTexture&& other) noexcept;
			 OGLTexture(const OGLTexture&) = delete;
			 OGLTexture& operator=(const OGLTexture&) = delete;

			~OGLTexture();

			static TextureBase* RGBATextureFromData(const Image& Image);

			static TextureBase* RGBATextureFromFilename(const std::string&name);

			// Begin TextureBase interface
			virtual Image GetRawTextureData() const override final;
			// End TextureBase interface

			static uint32 CalculateMipCount(int width, int height);
			void GenMipMaps();

			GLuint GetObjectID() const	{
				return texID;
			}

			// Return the source type (GL_RGB etc.) that should be used for the given number of channels
			// Defaults to GL_RGB
			static int DetermineSourceType(int channels);

		protected:						
			OGLTexture();

			GLuint texID;
			TextureConfig config;
		};
	}
}

