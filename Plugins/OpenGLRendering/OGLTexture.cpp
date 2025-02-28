/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "OGLTexture.h"
#include "OGLRenderer.h"
#include <Common.h>
#include "Common/Graphics/TextureLoader.h"
#include "Common/Math/Vector3.h"
#include "Core/Misc/Image.h"
#include <array>

using namespace NCL;
using namespace NCL::Rendering;

// Keep in sync with ImageType
constexpr std::array<GLint, static_cast<size_t>(ImageType::UNDEFINED)> glImageTypes = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
		GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY,
		GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

// Keep in sync with PixelFormat
constexpr std::array<GLint, static_cast<size_t>(PixelFormat::UNDEFINED)> glUploadFormats = {
	GL_RED,		GL_RG,		GL_RGB,			GL_RGBA,
	GL_BGR,		GL_BGRA,	GL_RED_INTEGER,	GL_RG_INTEGER,
	GL_RGB_INTEGER,	GL_RGBA_INTEGER, GL_BGR_INTEGER, GL_BGRA_INTEGER,
	GL_STENCIL_INDEX, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
};

constexpr auto textureStorageFunctions = std::array<void(*)(GLuint, const TextureConfig&), 9>{
	[](GLuint id, const TextureConfig& ci) { glTextureStorage1D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x); },
	[](GLuint id, const TextureConfig& ci) { glTextureStorage2D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.extent.y); },
	[](GLuint id, const TextureConfig& ci) { glTextureStorage3D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.extent.y, ci.extent.z); },
	[](GLuint id, const TextureConfig& ci) { glTextureStorage2D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.arrayLayers); }, // TEX_1D_ARRAY
	[](GLuint id, const TextureConfig& ci) { glTextureStorage3D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.extent.y, ci.arrayLayers); }, // TEX_2D_ARRAY
	[](GLuint id, const TextureConfig& ci) { glTextureStorage2D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.extent.y); }, // TEX_CUBEMAP
	[](GLuint id, const TextureConfig& ci) { glTextureStorage3D(id, ci.mipLevels, FormatToOGL(ci.format), ci.extent.x, ci.extent.y, ci.arrayLayers); }, // TEX_CUBEMAP_ARRAY
	[](GLuint id, const TextureConfig& ci) { glTextureStorage2DMultisample(id, ci.sampleCount, FormatToOGL(ci.format), ci.extent.x, ci.extent.y, GL_TRUE); }, // TEX_2D_MULTISAMPLE
	[](GLuint id, const TextureConfig& ci) { glTextureStorage3DMultisample(id, ci.sampleCount, FormatToOGL(ci.format), ci.extent.x, ci.extent.y, ci.arrayLayers, GL_TRUE); } // TEX_2D_MULTISAMPLE_ARRAY
};

// Keep in sync with ImageFormat
constexpr std::array<GLuint, static_cast<size_t>(ImageFormat::UNDEFINED)> glImageFormats = {
	GL_R8,             GL_R8_SNORM,         GL_R16,            GL_R16_SNORM,
	GL_RG8,            GL_RG8_SNORM,        GL_RG16,           GL_RG16_SNORM,
	GL_RGB8,           GL_RGB8_SNORM,       GL_RGB10,          GL_RGB12,
	GL_RGB16,          GL_RGB16_SNORM,      GL_RGBA2,          GL_RGBA4,
	GL_RGB5_A1,        GL_RGBA8,            GL_RGBA8_SNORM,    GL_RGB10_A2,
	GL_RGB10_A2UI,     GL_RGBA12,           GL_RGBA16,         GL_RGBA16_SNORM,
	GL_SRGB8,          GL_SRGB8_ALPHA8,     GL_R16F,           GL_RG16F,
	GL_RGB16F,         GL_RGBA16F,          GL_R32F,           GL_RG32F,
	GL_RGB32F,         GL_RGBA32F,          GL_R11F_G11F_B10F, GL_RGB9_E5,
	GL_R8I,            GL_R8UI,             GL_R16I,           GL_R16UI,
	GL_R32I,           GL_R32UI,            GL_RG8I,           GL_RG8UI,
	GL_RG16I,          GL_RG16UI,           GL_RG32I,          GL_RG32UI,
	GL_RGB8I,          GL_RGB8UI,           GL_RGB16I,         GL_RGB16UI,
	GL_RGB32I,         GL_RGB32UI,          GL_RGBA8I,         GL_RGBA8UI,
	GL_RGBA16I,        GL_RGBA16UI,         GL_RGBA32I,        GL_RGBA32UI,
	GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F,
	GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8, GL_STENCIL_INDEX1, GL_STENCIL_INDEX4,
	GL_STENCIL_INDEX8, GL_STENCIL_INDEX16, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1,
	GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
};

constexpr GLint NCL::Rendering::ImageTypeToOGL(ImageType imageType) {
	return glImageTypes[static_cast<size_t>(imageType)];
}

constexpr GLint NCL::Rendering::FormatToOGL(ImageFormat format) {
    return glImageFormats[static_cast<size_t>(format)];
}

constexpr void CreateTextureStorage(GLuint id, const TextureConfig& ci) {
	return textureStorageFunctions[static_cast<size_t>(ci.imageType)](id, ci);
}

OGLTexture::OGLTexture()
{
	glGenTextures(1, &texID);
}

OGLTexture::OGLTexture(GLuint texToOwn) {
	texID = texToOwn;
}

OGLTexture::OGLTexture(const TextureConfig& inConfig, std::string_view name) : config(inConfig){
	glCreateTextures(ImageTypeToOGL(config.imageType), 1, &texID);
	CreateTextureStorage(texID, config);

	if (!name.empty()) {
		glObjectLabel(GL_TEXTURE, texID, static_cast<GLsizei>(name.length()), name.data());
	}

	LOG_DEBUG("Created texture with handle: {}", texID);
}

OGLTexture& OGLTexture::operator=(OGLTexture&& other) noexcept {
	if (&other == this)
		return *this;
	this->~OGLTexture();
	return *new (this) OGLTexture(std::move(other));
}

OGLTexture::~OGLTexture()
{	
	if (texID == 0) return;

	glDeleteTextures(1, &texID);
	LOG_DEBUG("Destroyed texture with handle: {}", texID);
}

TextureBase* OGLTexture::RGBATextureFromData(const Image& image) {
	//OGLTexture* tex = new OGLTexture();

	int dataSize = image.GetPixelSize(); //This always assumes data is 1 byte per channel

	int sourceType = GL_RGB;

	switch (image.channels) {
		case 1: sourceType = GL_RED	; break;

		case 2: sourceType = GL_RG	; break;
		case 3: sourceType = GL_RGB	; break;
		case 4: sourceType = GL_RGBA; break;
		default:
			LOG_WARN("Could not determine source type for image");
		break;
	}

// OLD METHOD OF CREATING TEXTURES
//		glBindTexture(GL_TEXTURE_2D, tex->texID);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, x, y, 0, sourceType, GL_UNSIGNED_BYTE, data);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
////	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	glBindTexture(GL_TEXTURE_2D, 0);

	TextureConfig config{
		.imageType = ImageType::TEX_2D,
		.format = ImageFormat::RGB32F,
		.extent = {(int)image.width, (int)image.height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.sampleCount = 1,
	};

	OGLTexture* tex = new OGLTexture(config, "testTexture");
//	glTextureSubImage2D(tex->texID, 0, 0, 0, x, y, GL_UNSIGNED_BYTE, GL_RGBA, data);

	//glBindTexture(GL_TEXTURE_2D, tex->texID);
	// TODO: Decouple texture creation from sampler state
	glTextureParameteri(tex->texID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(tex->texID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(tex->texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(tex->texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureSubImage2D(tex->texID, 0, 0, 0, image.width, image.height, sourceType, GL_UNSIGNED_BYTE, image.data);

	tex->GenMipMaps();

	return tex;
}

TextureBase* OGLTexture::RGBATextureFromFilename(const std::string&name) {
	Image image;
	int flags		= 0;
	TextureLoader::LoadTexture(name, image, flags);  

	TextureBase* glTex = RGBATextureFromData(image);

	return glTex;
}

Image NCL::Rendering::OGLTexture::GetRawTextureData() const {
	auto [width, height, z] = config.extent.array;
	size_t bufferSize = width * height * z * 4;
	// The returned Image will take ownership of the memory.
	uint8* buffer = (uint8*)std::malloc(bufferSize);
	// TODO Assuming 4 channels, need to refactor at some point and store sourceType somewhere;
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTextureImage(texID, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferSize, (void*)buffer);
	return Image(buffer, width, height, 4);
}

int OGLTexture::CalculateMipCount(int width, int height) {
	return (int)floor(log2(float(std::min(width, height)))) + 1;
}

void OGLTexture::GenMipMaps() {
	glGenerateTextureMipmap(texID);
}
