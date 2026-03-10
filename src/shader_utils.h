#pragma once

#include <string>
#include <glad/glad.h>

namespace ShaderUtils {
    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram(const char* vertSrc, const char* fragSrc);
    std::string loadFile(const std::string& path);
}
