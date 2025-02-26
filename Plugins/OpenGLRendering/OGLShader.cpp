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
#include <set>

#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include "Common/stb/stb_include.h"

using namespace NCL;
using namespace NCL::Rendering;
using namespace NCL::Maths;

constexpr GLuint shaderTypes[(int)ShaderStages::SHADER_MAX] = {
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
	"Tess. Eval",
	"Compute"
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
	for (int i = 0;  string& shader : shaderFiles) {
		if (!shader.empty()) {
			if (Assets::ReadTextFile(Assets::SHADERDIR + shader, fileContents)) {

				char error[256]{};

				auto processed_ptr = std::unique_ptr<char, decltype([](char* p) {free(p); })>(
					stb_include_string(fileContents.c_str(), nullptr, Assets::SHADERDIR.c_str(), nullptr, error));
				if (!processed_ptr) {
					std::cerr << "Failed to process includes." << std::endl;
					return;
				}

				std::string_view processedContents(processed_ptr.get());
				shaderIDs[i] = glCreateShader(shaderTypes[i]);

				LOG_INFO("Reading {} shader {}", ShaderNames[i], shader);

				const char* stringData	 = processedContents.data();
				int			stringLength = (int)processedContents.length();
				glShaderSource(shaderIDs[i], 1, &stringData, &stringLength);
				glCompileShader(shaderIDs[i]);

				glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &shaderValid[i]);
		
				if (shaderValid[i] != GL_TRUE) {
					LOG_INFO("{} shader {} has failed", ShaderNames[i], shader);
				}
				else {
					glAttachShader(programID, shaderIDs[i]);
				}
				PrintCompileLog(shaderIDs[i]);
			}
		}
		++i;
	}	

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

	PrintLinkLog(programID);

	if (programValid != GL_TRUE) {
		LOG_ERROR("This shader has failed");
	}
	else {
		CacheUniforms();
		LOG_INFO("This shader has loaded");
	}
}

OGLShader::OGLShader() : programID(0), programValid(0) {
	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		shaderIDs[i] = 0;
		shaderValid[i] = 0;
	}
}

void OGLShader::AddBinaryShaderModule(const string& fromFile, ShaderStages stage) {
	shaderFiles[(int)stage] = fromFile;
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
		LOG_INFO("Compile Log:\n {}", tempData);
		delete[] tempData;
	}
}

void OGLShader::PrintLinkLog(GLuint program) {
	int logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength) {
		char* tempData = new char[logLength];
		glGetProgramInfoLog(program, logLength, NULL, tempData);
		LOG_INFO("Link Log:\n{}", tempData);
		delete[] tempData;
	}
}

void OGLShader::PrintUniformCache() {
	LOG_INFO("Uniform Log:\n{}");
	for (auto& [name, entry] : uniformCache) {
		LOG_INFO("Name: {}", name);
		LOG_INFO("\tLocation: {}", entry.location);
		LOG_INFO("\tCount: {}", entry.count);
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
	auto it = uniformCache.find(name);
	if (uniformCache.find(name) != uniformCache.end()) {
		return { it->second };
	}
	return {};
}

bool OGLShader::HasUniformEntry(const std::string& name) const {
	return uniformCache.contains(name);
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

OGLShaderBuilder& OGLShaderBuilder::With(ShaderStages stage, const std::string& shaderPath) {
	shaderFiles[(int)stage] = shaderPath;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithVertex(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_VERTEX] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithFragment(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_FRAGMENT] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithGeometry(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_GEOMETRY] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithTessControl(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_DOMAIN] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithTessEval(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_HULL] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithCompute(const string& name) {
	shaderFiles[(int)ShaderStages::SHADER_COMPUTE] = name;
	return *this;
}

OGLShaderBuilder& OGLShaderBuilder::WithDebugName(const string& name) {
	debugName = name;
	return *this;
}

std::optional<OGLShader*> OGLShaderBuilder::Build() {
	OGLShader* newShader = new OGLShader();

	for (int i = 0; i < (int)ShaderStages::SHADER_MAX; ++i) {
		if (!shaderFiles[i].empty()) {
			newShader->AddBinaryShaderModule(shaderFiles[i], (ShaderStages)i);

			//if (!debugName.empty()) {
			//	renderer.SetDebugName(vk::ObjectType::eShaderModule, (uint64_t)newShader->shaderModules[i].operator VkShaderModule(), debugName);
			//}
		}
	};
	newShader->ReloadShader();
	return { newShader };
}

int OGLShaderBuilder::CompileStage(const uint32_t id, const std::string& shaderCode) {
	return 0;
}

void OGLShaderBuilder::Compile(const uint32_t id, const char* shaderCode) {
	glShaderSource(id, 1, &shaderCode, nullptr);
	glCompileShader(id);
}

int OGLShaderBuilder::Link(const uint32_t id) {
	int success = 0;
	char infoLog[512];

	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	glGetProgramInfoLog(id, 512, NULL, infoLog);

	if (!success) {
		std::cerr << "Failed to link shader: " << infoLog << std::endl;
	}

	return success;
}

int OGLShaderBuilder::Validate(const uint32_t id) {
	int success = 0;
	char infoLog[512];

	glValidateProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	glGetProgramInfoLog(id, 512, NULL, infoLog);

	if (!success) {
		std::cerr << "Failed to validate shader: " << infoLog << std::endl;
	}

	return success;
}
