#pragma once
#include "NCLAliases.h"
#include "FunctionUtils.h"
#include <cstdint>
#include <cstdlib>

namespace NCL {

	/* Wrapper around raw image data */
	class Image : public NonCopyable {
	public:

		Image() = default;
		Image(uint8* data, size_t width, size_t height, uint8 channels) : 
			data(data), width(width), height(height), channels(channels) {}	
		~Image() { 
			Free(); 
		}

		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;

		void Free() { 
			NCL_SAFE_FREE(data);
		}

		size_t GetPixelSize() const {
			return width * height * channels;
		}

		uint8* data = nullptr;
		size_t width = 0;
		size_t height = 0;
		uint8 channels = 0;
	};
}

