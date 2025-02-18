/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "OGLShader.h"
#include "Common/Resources/Assets.h"
#include "Common/Math/Maths.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <memory>

#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include "Common/stb/stb_include.h"

using namespace NCL;
using namespace NCL::Rendering;
using namespace NCL::Maths;

GLuint shaderTypes[(int)ShaderStages::SHADER_MAX] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_COMPUTE_SHADER,
};

string ShaderNames[(int)ShaderStages::SHADER_MAX] = {
	"Vertex",
	"Fragment",
	"Geometry",
	"Tess. Control",
	"Tess. Eval"
};

OGLShader::OGLShader(const string& vertex, const string& fragment, const string& geometry, const string& domain, const string& hull) :
	ShaderBase(vertex, fragment, geometry, domain, hull) {

	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		shaderIDs[i]	= 0;
		shaderValid[i]	= 0;
	}
	programID = 0;

	ReloadShader();
}

OGLShader::OGLShader(const string& compute) : ShaderBase(compute) {
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		shaderIDs[i] = 0;
		shaderValid[i] = 0;
	}
	programID = 0;

	ReloadShader();
}

OGLShader::~OGLShader()	{
	DeleteIDs();
}

void OGLShader::ReloadShader() {
	DeleteIDs();
	ClearCache();
	programID = glCreateProgram();
	string fileContents = "";
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		if (!shaderFiles[i].empty()) {
			if (Assets::ReadTextFile(Assets::SHADERDIR + shaderFiles[i], fileContents)) {

				char error[256]{};

				auto processed_ptr = std::unique_ptr<char, decltype([](char* p) {free(p); })>(
					stb_include_string(fileContents.c_str(), nullptr, Assets::SHADERDIR.c_str(), nullptr, error));
				if (!processed_ptr) {
					std::cerr << "Failed to process includes." << std::endl;
					return;
				}

				std::string_view processedContents(processed_ptr.get());
				shaderIDs[i] = glCreateShader(shaderTypes[i]);

				std::cout << "Reading " << ShaderNames[i] << " shader " << shaderFiles[i] << std::endl;

				const char* stringData	 = processedContents.data();
				int			stringLength = (int)processedContents.length();
				glShaderSource(shaderIDs[i], 1, &stringData, &stringLength);
				glCompileShader(shaderIDs[i]);

				glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &shaderValid[i]);
		
				if (shaderValid[i] != GL_TRUE) {
					std::cout << ShaderNames[i] << " shader " << " has failed!" << std::endl;
				}
				else {
					glAttachShader(programID, shaderIDs[i]);
				}
				PrintCompileLog(shaderIDs[i]);
			}
		}
	}	
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

	PrintLinkLog(programID);

	if (programValid != GL_TRUE) {
		std::cout << "This shader has failed!" << std::endl;
	}
	else {
		CacheUniforms();
		std::cout << "Shader loaded!" << std::endl;
	}
}

void OGLShader::DeleteIDs() {
	if (!programID) {
		return;
	}
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		if (shaderIDs[i]) {
			glDetachShader(programID, shaderIDs[i]);
			glDeleteShader(shaderIDs[i]);
		}
	}
	glDeleteProgram(programID);
	programID = 0;
}

void	OGLShader::PrintCompileLog(GLuint object) {
	int logLength = 0;
	glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength) {
		char* tempData = new char[logLength];
		glGetShaderInfoLog(object, logLength, NULL, tempData);
		std::cout << "Compile Log:\n" << tempData << std::endl;
		delete[] tempData;
	}
}

void OGLShader::PrintLinkLog(GLuint program) {
	int logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength) {
		char* tempData = new char[logLength];
		glGetProgramInfoLog(program, logLength, NULL, tempData);
		std::cout << "Link Log:\n" << tempData << std::endl;
		delete[] tempData;
	}
}

GLint OGLShader::GetUniformLocation(const std::string& name) const {
	auto it = uniformCache.find(name);
	if (uniformCache.find(name) != uniformCache.end()) {
		return it->second.location;
	}

	GLint location = glGetUniformLocation(programID, name.c_str());
	uniformCache[name].location = location;
	return location;
}

std::optional<UniformEntry> OGLShader::GetUniformEntry(const std::string& name) const {
	/*auto it = uniformCache.find(name);
	if (uniformCache.find(name) != uniformCache.end()) {
		return { it->second };
	}
	return {};*/
	return {};
}

// From Guide to Modern OpenGL
void OGLShader::CacheUniforms() {
	GLint uniform_count = 0;

	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniform_count);

	if (uniform_count != 0) {
		GLint 	maxNameLen = 0;
		GLsizei length = 0;
		GLsizei count = 0;
		GLenum 	type = GL_NONE;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLen);

		auto uniform_name = std::make_unique<char[]>(maxNameLen);

		for (GLint i = 0; i < uniform_count; ++i) {
			glGetActiveUniform(programID, i, maxNameLen, &length, &count, &type, uniform_name.get());

			UniformEntry uniform_info = {};
			uniform_info.location = glGetUniformLocation(programID, uniform_name.get());
			uniform_info.count = count;

			uniformCache.emplace(std::make_pair(std::string(uniform_name.get(), length), uniform_info));
		}
	}
}
