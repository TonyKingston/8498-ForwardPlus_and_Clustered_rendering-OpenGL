#pragma once
#include "NCLAliases.h"

namespace NCL {
	namespace Rendering {
        class TextureBase
        {
        public:
            virtual ~TextureBase();
        protected:
            TextureBase();

        };

		enum class ImageType : uint32 {
			TEX_1D,
			TEX_2D,
			TEX_3D,
			TEX_1D_ARRAY,
			TEX_2D_ARRAY,
			TEX_CUBEMAP,
			TEX_CUBEMAP_ARRAY,
			TEX_2D_MULTISAMPLE,
			TEX_2D_MULTISAMPLE_ARRAY,
		};

		enum class ImageFormat : uint32 {
            R8,
            R8_SNORM,
            R16,
            R16_SNORM,
            RG8,
            RG8_SNORM,
            RG16,
            RG16_SNORM,
            RGB8,
            RGB8_SNORM,
            RGB10,
            RGB12,
            RGB16,
            RGB16_SNORM,
            RGBA2,
            RGBA4,
            RGB5_A1,
            RGBA8,
            RGBA8_SNORM,
            RGB10_A2,
            RGB10_A2UI,
            RGBA12,
            RGBA16,
            RGBA16_SNORM,
            SRGB8,
            SRGB8_ALPHA8,
            R16F,
            RG16F,
            RGB16F,
            RGBA16F,
            R32F,
            RG32F,
            RGB32F,
            RGBA32F,
            R11F_G11F_B10F,
            RGB9_E5,
            R8I,
            R8UI,
            R16I,
            R16UI,
            R32I,
            R32UI,
            RG8I,
            RG8UI,
            RG16I,
            RG16UI,
            RG32I,
            RG32UI,
            RGB8I,
            RGB8UI,
            RGB16I,
            RGB16UI,
            RGB32I,
            RGB32UI,
            RGBA8I,
            RGBA8UI,
            RGBA16I,
            RGBA16UI,
            RGBA32I,
            RGBA32UI,

            // Depth & Stencil
            DEPTH_COMPONENT16,
            DEPTH_COMPONENT24,
            DEPTH_COMPONENT32,
            DEPTH_COMPONENT32F,
            DEPTH24_STENCIL8,
            DEPTH32F_STENCIL8,
            STENCIL_INDEX1,
            STENCIL_INDEX4,
            STENCIL_INDEX8,
            STENCIL_INDEX16,

            COMPRESSED_RGB_S3TC_DXT1_EXT,
            COMPRESSED_RGBA_S3TC_DXT1_EXT,
            COMPRESSED_RGBA_S3TC_DXT3_EXT,
            COMPRESSED_RGBA_S3TC_DXT5_EXT,
            COMPRESSED_RED_RGTC1,
            COMPRESSED_SIGNED_RED_RGTC1,
            COMPRESSED_RG_RGTC2,
            COMPRESSED_SIGNED_RG_RGTC2,
            COMPRESSED_RGBA_BPTC_UNORM,
            COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
            COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
            COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,

            UNDEFINED,
		};
	}
}

