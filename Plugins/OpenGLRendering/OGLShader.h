#pragma once
#include "Common/Graphics/ShaderBase.h"
#include "Common/Math/Maths.h"
#include "glad\glad.h"
#include <unordered_map>
#include <string>
#include <variant>
#include <utility>
#include <type_traits>

namespace NCL {
	namespace Rendering {

		//#define SET_UNIFORMS(shader, ...) OGLShader::SetUniforms(shader, { __VA_ARGS__ })

		struct UniformEntry {
			GLint location;
			// Equals number of elements for an array or 1
			GLint count;
		};

		class OGLShader : public ShaderBase
		{
		public:
			friend class OGLRenderer;
			OGLShader(const string& vertex, const string& fragment, const string& geometry = "", const string& domain = "", const string& hull = "");
			OGLShader(const string& compute);
			~OGLShader();

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
				if constexpr (std::is_same_v<T, int>) {
					glUniform1i(GetUniformLocation(name), value);
				}
				else if constexpr (std::is_same_v<T, float>) {
					glUniform1f(GetUniformLocation(name), value);
				}
			}

			//template <typename T, typename = std::enable_if_t<IsVector<T>::value>>
			template< typename VecType>
			std::enable_if_t<NCL::Maths::IsVector<VecType>::value, void> SetUniform(const std::string& name, const VecType& value) const {
				constexpr int VecSize = sizeof(VecType) / sizeof(float);

				if constexpr (VecSize == 2) {
					glUniform2fv(GetUniformLocation(name), 1, (float*)&value);
				}
				else if constexpr (VecSize == 3) {
					glUniform3fv(GetUniformLocation(name), 1, (float*)&value);
				}
				else if constexpr (VecSize == 4) {
					glUniform4fv(GetUniformLocation(name), 1, (float*)&value);
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

				std::cout << "Not implemented just yet" << std::endl;
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

				SetUniformsImpl(shader, uniformPairs, std::make_index_sequence<numPairs>{});

				// I think this would be C++ 20 version. Need to test:
			/*	auto apply = [&] <std::size_t... I>(std::index_sequence<I...>) {
					(shader->SetUniform(std::get<I * 2>(uniformPairs), std::get<I * 2 + 1>(uniformPairs)), ...);
				}(std::make_index_sequence<numPairs>{});

				apply(std::make_index_sequence<numPairs>{});*/
			}

			static void	PrintCompileLog(GLuint object);
			static void	PrintLinkLog(GLuint program);

		protected:
			void	DeleteIDs();
			GLuint	programID;
			GLuint	shaderIDs[(int)ShaderStages::SHADER_MAX];
			int		shaderValid[(int)ShaderStages::SHADER_MAX];
			int		programValid;

			//mutable std::unordered_map<std::string, GLint> uniformCache;
			mutable std::unordered_map<std::string, UniformEntry> uniformCache;

			GLint GetUniformLocation(const std::string& name) const;
			
			void CacheUniforms();

		private:

			// Helper function to apply the uniforms since I'm not using C++ 20 (no templated lambdas)
			template <typename Tuple, std::size_t... I>
			static void SetUniformsImpl(OGLShader* shader, const Tuple& uniformPairs, std::index_sequence<I...>) {
				(shader->SetUniform(std::get<I * 2>(uniformPairs), std::get<I * 2 + 1>(uniformPairs)), ...);
			}
		};
	}
}