#pragma once
#include "NCLAliases.h"
#include "FunctionUtils.h"
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <span>
#include <variant>

namespace NCL {

	/* Wrapper around raw image data */
	class Image : public NonCopyable {
		// Supporting floats for HDR files
		// TODO: Might add nullptr_t
		using ImageData = std::variant<uint8_t*, float*>;
	public:

		Image() = default;
		Image(uint8* data, size_t width, size_t height, uint8 channels) :
			data(data), width(width), height(height), channels(channels) {}

		Image(float* data, size_t width, size_t height, uint8 channels) :
			data(data), width(width), height(height), channels(channels) {}
		
		// Uses stbi_load to load image at path 
		explicit Image(const std::filesystem::path& path, int desiredChannels = 4);
		~Image() { Free(); }

		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;

		explicit operator bool() const { IsValid(); }

		bool IsValid() const {
			return std::visit([](auto ptr) { return ptr != nullptr; }, data);
		}

		void Free() { 
			std::visit([](auto ptr) {
				NCL_SAFE_FREE(ptr);
			}, data);
			data = static_cast<uint8_t*>(nullptr);
		}

		[[nodiscard]] size_t GetChannelSize() const {
			return IsFloat() ? sizeof(float) : sizeof(uint8_t);
		}

		[[nodiscard]] size_t GetPixelSize() const {
			return GetChannelSize() * channels;
		}
		[[nodiscard]] size_t GetTotalSize() const {
			return width * height * GetPixelSize();
		}

		template <typename T>
		[[nodiscard]] std::span<const T> Pixels() const {
			return std::visit([&](auto* ptr) -> std::span<const T> {
				using Actual = std::remove_pointer_t<decltype(ptr)>;
				if constexpr (std::is_same_v<Actual, T>) {
					return { ptr, GetTotalSize() };
				}
				else {
					return {}; 
				}
			}, data);
		}

		template <typename T>
		[[nodiscard]] const T* Data() const {
			return std::visit([](auto* ptr) -> const T* {
				using Actual = std::remove_pointer_t<decltype(ptr)>;
				if constexpr (std::is_same_v<Actual, T>) {
					return ptr;
				}
				else {
					return nullptr;
				}
			}, data);
		}

		// Returns an immutable view over the pixel data. Assumes byte format
		[[nodiscard]] std::span<const uint8> Pixels() const {
			if (!IsFloat()) {
				return { std::get<uint8_t*>(data), GetTotalSize() };
			}
			return {};
		}

		// Returns an immutable view over the pixel data. Assumes float format
		[[nodiscard]] std::span<const float> FloatPixels() const {
			if (IsFloat()) {
				return { std::get<float*>(data), GetTotalSize()};
			}
			return {};
		}

		// Returns an immutable pointer to the pixel data. Null if the data is actually floating point
		[[nodiscard]] const uint8* Data() const { 
			return !IsFloat() ? std::get<uint8_t*>(data) : nullptr; 
		}

		// Returns an immutable pointer to the pixel data. Null if the data is not floating point
		[[nodiscard]] const float* FloatData() const {
			return IsFloat() ? std::get<float*>(data) : nullptr;
		}

		[[nodiscard]] size_t RowPitch() const {
			return width * GetPixelSize();
		}

		[[nodiscard]] size_t PixelOffset(size_t x, size_t y) const {
			NCL_ASSERT(x < width && y < height);
			return y * RowPitch() + x * GetPixelSize();
		}

		[[nodiscard]] int Width() const { return width; }
		[[nodiscard]] int Height() const { return height; }
		[[nodiscard]] int Channels() const { return channels; }
		[[nodiscard]] bool IsFloat() const { return std::holds_alternative<float*>(data); }

	private:
		void LoadInternal(const char* filename, int desiredChannels, bool loadAsFloat);
		void Nullify() noexcept;

		ImageData data = static_cast<uint8*>(nullptr);
		size_t width = 0;
		size_t height = 0;
		uint8 channels = 0;
	};
}

