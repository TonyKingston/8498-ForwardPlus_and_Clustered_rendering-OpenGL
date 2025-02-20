#pragma once
#include <string>
#include <memory>
#include <array>

using std::string;
namespace NCL {
	namespace Rendering {
		enum class ShaderStages {
			SHADER_VERTEX,
			SHADER_FRAGMENT,
			SHADER_GEOMETRY,
			SHADER_DOMAIN,
			SHADER_HULL,
			SHADER_COMPUTE,
			SHADER_MAX
		};

		class ShaderBase	{
		public:
			ShaderBase() = default;
			ShaderBase(const string& vertex, const string& fragment, const string& geometry = "", const string& domain = "", const string& hull = "");
			ShaderBase(const string& compute);
			virtual ~ShaderBase() = default;

			virtual void ReloadShader() = 0;
		protected:

			ShaderBase(ShaderBase&& other) noexcept {
				std::move(std::begin(other.shaderFiles), std::end(other.shaderFiles), std::begin(shaderFiles));
			};

			ShaderBase& operator=(ShaderBase&& other) noexcept {
				std::move(std::begin(other.shaderFiles), std::end(other.shaderFiles), std::begin(shaderFiles));
				return *this;
			};

			std::array<string, (int)ShaderStages::SHADER_MAX> shaderFiles;
		};
	}
}

