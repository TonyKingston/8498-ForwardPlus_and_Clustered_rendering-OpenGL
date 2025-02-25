#pragma once
#include "Common/Graphics/ShaderBase.h"
#include "Common/Math/Maths.h"
#include "Common/Macros.h"
#include "glad\glad.h"
#include <unordered_map>
#include <string>
#include <variant>
#include <utility>
#include <type_traits>
#include <optional>
#include <array>
#include <span>

namespace NCL {
	namespace Rendering {

		//#define SET_UNIFORMS(shader, ...) OGLShader::SetUniforms(shader, { __VA_ARGS__ })

		// Represents a cached uniform
		struct UniformEntry {
			GLint location;
			// Equals number of elements for an array otherwise 1 (for a singular vec3 etc.)
			GLint count;
		};

		class OGLShader : public ShaderBase
		{
		public:
			friend class OGLRenderer;
			friend class OGLShaderBuilder;

			OGLShader(const string& vertex, const string& fragment, const string& geometry = "", const string& domain = "", const string& hull = "");
			OGLShader(const string& compute);
			OGLShader(OGLShader& other) = delete;
			OGLShader& operator=(OGLShader& other) = delete;
			~OGLShader();

			OGLShader(OGLShader&& other) noexcept :
				ShaderBase(std::move(other)),
				programID(std::exchange(other.programID, 0)),
				programValid(std::exchange(other.programValid, 0)),
				uniformCache(std::move(other.uniformCache))
			{
				std::move(std::begin(other.shaderIDs), std::end(other.shaderIDs), std::begin(shaderIDs));
				std::move(std::begin(other.shaderValid), std::end(other.shaderValid), std::begin(shaderValid));
			}

			OGLShader& operator=(OGLShader&& other) noexcept {
				ShaderBase::operator=(std::move(other));
				programID = std::exchange(other.programID, 0);
				programValid = std::exchange(other.programValid, 0);
				uniformCache = std::move(other.uniformCache);
				std::move(std::begin(other.shaderIDs), std::end(other.shaderIDs), std::begin(shaderIDs));
				std::move(std::begin(other.shaderValid), std::end(other.shaderValid), std::begin(shaderValid));
				return *this;
			}

			void ReloadShader() override;

			void ClearCache() { uniformCache.clear(); }

			bool LoadSuccess() const {
				return programValid == GL_TRUE;
			}	

			int GetProgramID() const {
				return programID;
			}	

			template <typename T>
			std::enable_if_t<std::is_arithmetic_v<T>, void> SetUniform(const std::string& name, const T& value) const
			{
				if constexpr (std::is_integral_v<T>) {
					glUniform1i(GetUniformLocation(name), value);
				}	
				else if constexpr (std::is_floating_point_v<T>) {
					glUniform1f(GetUniformLocation(name), value);
				}
			}

			//template <typename T, typename = std::enable_if_t<IsVector<T>::value>>
			template< typename VecType>
			std::enable_if_t<NCL::Maths::IsVector<VecType>::value, void> SetUniform(const std::string& name, const VecType& value) const {
				constexpr int VecSize = sizeof(VecType) / sizeof(float);
				static_assert(VecSize >= 2 && VecSize <= 4, "Unsupported vector size");

				std::span<const float> vecSpan(reinterpret_cast<const float*>(&value), VecSize);

				if constexpr (VecSize == 2) {
					glUniform2fv(GetUniformLocation(name), 1, vecSpan.data());
				}
				else if constexpr (VecSize == 3) {
					glUniform3fv(GetUniformLocation(name), 1, vecSpan.data());
				}
				else if constexpr (VecSize == 4) {
					glUniform4fv(GetUniformLocation(name), 1, vecSpan.data());
				}
			}

			template <typename MatType>
			std::enable_if_t<std::is_same_v<MatType, NCL::Maths::Matrix4>, void> SetUniform(const std::string& name, const MatType& value) const {
				constexpr int MatSize = sizeof(MatType) / sizeof(float);

				if constexpr (MatSize == 16) {
					glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value.array);
				}
			}

			//template <typename... Args, typename = std::enable_if_t<(std::is_arithmetic_v<Args> && ...)>>
			template <typename... Args>
			std::enable_if_t<(std::is_arithmetic_v<Args> && ...), void> SetVectorUniform(const std::string& name, Args... args) const {
				static_assert(sizeof...(args) >= 1 && sizeof...(args) <= 4, "SetUniform requires 1 to 4 arguments");

				LOG_INFO("{} Not implemented just yet", __FUNCTION__);
			}

			/* Apply multiple uniforms without having to repeatedly call SetUniform.
			args: Takes a sequence of strings and uniform values e.g.
			"tilePxX", clusterX,
			"tilePxY", clusterY 
			Precondition: args must have an even number of arguments*/
			template <typename... Args>
			static void SetUniforms(OGLShader* shader, Args&&... args) {
				static_assert(sizeof...(args) % 2 == 0, "SetUniforms requires an even number of arguments (name, value pairs)");

				auto uniformPairs = std::make_tuple(std::forward<Args>(args)...);
				constexpr std::size_t numPairs = sizeof...(args) / 2;

				auto applyUniforms = [&]<typename std::size_t... I>(std::index_sequence<I...>) {
					(shader->SetUniform(std::get<I * 2>(uniformPairs), std::get<I * 2 + 1>(uniformPairs)), ...);
				};

				applyUniforms(std::make_index_sequence<numPairs>{});
			}

			static void	PrintCompileLog(GLuint object);
			static void	PrintLinkLog(GLuint program);
			void PrintUniformCache();

		protected:
			OGLShader();

			void AddBinaryShaderModule(const string& fromFile, ShaderStages stage);
			//static bool CreateShaderModule(char* data, size_t size, vk::ShaderModule& into, vk::Device& device);

			void	DeleteIDs();
			GLuint	programID;
			GLuint	shaderIDs[(int)ShaderStages::SHADER_MAX];
			int		shaderValid[(int)ShaderStages::SHADER_MAX];
			int		programValid;

			//mutable std::unordered_map<std::string, GLint> uniformCache;
			mutable std::unordered_map<std::string, UniformEntry> uniformCache;

			GLint GetUniformLocation(const std::string& name) const;

			// @return If found, a uniform entry struct containing the location and size of the uniform.
			std::optional<UniformEntry> GetUniformEntry(const std::string& name) const;

			// Returns true, if a uniform with name exists in the shader
			// Call GetUniformEntry is you actually want to retrieve info about the uniform.
			bool HasUniformEntry(const std::string& name) const;
			
			void CacheUniforms();

		private:

			// Helper function to apply the uniforms since (pre C++ 20 i.e. no templated lambdas)
			// Not used anymore.
			template <typename Tuple, std::size_t... I>
			[[deprecated("This shouldn't be needed for C++20 onwards")]]
			static void SetUniformsImpl(OGLShader* shader, const Tuple& uniformPairs, std::index_sequence<I...>) {
				(shader->SetUniform(std::get<I * 2>(uniformPairs), std::get<I * 2 + 1>(uniformPairs)), ...);
			}
		};

		class OGLShaderBuilder
		{
		public:
			OGLShaderBuilder() = default;
			~OGLShaderBuilder() = default;

			OGLShaderBuilder& With(ShaderStages stage, const std::string& shaderPath);
			OGLShaderBuilder& WithVertex(const string& name);
			OGLShaderBuilder& WithFragment(const string& name);
			OGLShaderBuilder& WithGeometry(const string& name);
			OGLShaderBuilder& WithTessControl(const string& name);
			OGLShaderBuilder& WithTessEval(const string& name);
			OGLShaderBuilder& WithCompute(const string& name);

			/* Usage example: .With({SHADER_VERTEX, "shader.vert"}, {SHADER_FRAGMENT, "shader.frag"} */
			OGLShaderBuilder& With(std::initializer_list<std::pair<ShaderStages, std::string>> shaders) {
				NCL_ASSERT(shaders.size() <= (int)ShaderStages::SHADER_MAX);
				for (const auto& [stage, path] : shaders) {
					With(stage, path);
				}
				return *this;
			}

			OGLShaderBuilder& WithDebugName(const string& name);

			std::optional<OGLShader*> Build();

		private:
			// TODO: CompileStage is Not implemented
			int CompileStage(const uint32_t id, const std::string& shaderCode);
			void Compile(const uint32_t id, const char* shaderCode);

			int Link(const uint32_t id);
			int Validate(const uint32_t id);

		private:
			std::array<string, (int)ShaderStages::SHADER_MAX> shaderFiles;
			string debugName;

		};
	}
}