/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Common/Graphics/TextureBase.h"
#include "Common/Math/Maths.h"
#include "glad\glad.h"
#include <string>

using namespace NCL::Maths;

namespace NCL {
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

			static TextureBase* RGBATextureFromData(char* data, int width, int height, int channels);

			static TextureBase* RGBATextureFromFilename(const std::string&name);

			static int CalculateMipCount(int width, int height);
			void GenMipMaps();

			GLuint GetObjectID() const	{
				return texID;
			}
		protected:						
			OGLTexture();

			GLuint texID;
			TextureConfig config;
		};
	}
}

