#include "shader.hpp"

#include "../helper/OGLImport.hpp"
#include <iostream>

int Shader::getUniformLocation(const std::string & name)
{
    checkOGL(int location = glGetUniformLocation(m_RendererId, name.c_str()));
    if (location == -1) std::cout << "Warning: uniform '" << name << "' doesnt exist!\n";
    return location;
}

unsigned int Shader::compileShader(unsigned int shaderType, const std::string& source)
{
    checkOGL(unsigned int id = glCreateShader(shaderType));
    const char* src = source.c_str();
    checkOGL(glShaderSource(id, 1, &src, nullptr));
    checkOGL(glCompileShader(id));

    int result;
    checkOGL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        checkOGL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        checkOGL(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "failded to compile " << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader\n";
        std::cout << message + '\n';
        checkOGL(glDeleteShader(id));
        return 0;
    }
    return id;
}

unsigned int Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    checkOGL(unsigned int program = glCreateProgram());
    unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    checkOGL(glAttachShader(program, vertShader));
    checkOGL(glAttachShader(program, fragShader));
    checkOGL(glLinkProgram(program));
    checkOGL(glValidateProgram(program));

    checkOGL(glDeleteShader(vertShader));
    checkOGL(glDeleteShader(fragShader));

    return program;
}

Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    m_RendererId = createShader(vertexShaderPath, fragmentShaderPath);
}

Shader::Shader(Shader&& other)
{
    m_RendererId = other.m_RendererId;

    other.m_RendererId = 0;
}

void Shader::bind() const
{
    checkOGL(glUseProgram(m_RendererId));
}

void Shader::unbind() const
{
    checkOGL(glUseProgram(0));
}

void Shader::destroy()
{
    checkOGL(glDeleteProgram(m_RendererId));
}

void Shader::setUniform1i(const std::string& name, int value)
{
    checkOGL(glUniform1i(getUniformLocation(name), value)); // cache uniform location for update loops!
}

void Shader::setUniform4f(const std::string& name, float f1, float f2, float f3, float f4)
{
    checkOGL(glUniform4f(getUniformLocation(name), f1, f2, f3, f4)); // cache uniform location for update loops!
}

void Shader::setUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    checkOGL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0])); // cache uniform location for update loops!
}