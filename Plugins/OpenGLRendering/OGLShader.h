#pragma once
#include "../../Common/ShaderBase.h"
#include "../../Common/Maths.h"
#include "glad\glad.h"
#include <unordered_map>
#include <string>

namespace NCL {
	namespace Rendering {
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
			std::enable_if_t<std::is_arithmetic_v<T>, void> SetUniform(const std::string& name, const T& value) const;

			//template <typename T, typename = std::enable_if_t<IsVector<T>::value>>
			template< typename VecType>
			std::enable_if_t<NCL::Maths::IsVector<VecType>::value, void> SetUniform(const std::string& name, const VecType& value) const;

			template <typename MatType>
			std::enable_if_t<std::is_same_v<MatType, NCL::Maths::Matrix4>, void> SetUniform(const std::string& name, const MatType& value) const;

			//template <typename... Args, typename = std::enable_if_t<(std::is_arithmetic_v<Args> && ...)>>
			template <typename... Args>
			std::enable_if_t<(std::is_arithmetic_v<Args> && ...), void> SetVectorUniform(const std::string& name, Args... args) const;
			
			static void	PrintCompileLog(GLuint object);
			static void	PrintLinkLog(GLuint program);

		protected:
			void	DeleteIDs();
			GLuint	programID;
			GLuint	shaderIDs[(int)ShaderStages::SHADER_MAX];
			int		shaderValid[(int)ShaderStages::SHADER_MAX];
			int		programValid;

			mutable std::unordered_map<std::string, GLint> uniformCache;

			GLint GetUniformLocation(const std::string& name) const;
		};
	}
}