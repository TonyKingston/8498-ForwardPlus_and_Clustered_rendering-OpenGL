#include "pch.h"
#include "TextureBase.h"
#include "Core/Misc/Image.h"
using namespace NCL;
using namespace Rendering;

TextureBase::TextureBase()
{
}


TextureBase::~TextureBase()
{
}

Image NCL::Rendering::TextureBase::GetRawTextureData() const {
	LOG_WARN("GetRawTextureData is unimplemented");
	return Image();
}
