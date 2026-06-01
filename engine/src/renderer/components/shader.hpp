#pragma once
#include <string>
#include <glm.hpp>

class Shader
{
	unsigned int m_RendererId;

	int getUniformLocation(const std::string & name);
	unsigned int compileShader(unsigned int shaderType, const std::string& source);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);

public:
	Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Shader(Shader&& other);

	void bind() const;
	void unbind() const;
	void destroy();

	void setUniform1i(const std::string& name, int value);
	void setUniform4f(const std::string& name, float f1, float f2, float f3, float f4);
	void setUniformMat4f(const std::string& name, const glm::mat4& matrix);

};