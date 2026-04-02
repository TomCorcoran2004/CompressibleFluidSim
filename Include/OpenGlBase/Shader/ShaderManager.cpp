#include "ShaderManager.h"
#include <iostream>

namespace Base
{
    bool ShaderProgram::Init(const ShaderProgramConfig& Config)
    {        
        assert(Config.VertexSource);
        assert(Config.FragmentSource);

        GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShader, 1, &Config.VertexSource, nullptr);
        glCompileShader(VertexShader);
        assert(LogShaderCompilationStatus(VertexShader));

        GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShader, 1, &Config.FragmentSource, nullptr);
        glCompileShader(FragmentShader);
        assert(LogShaderCompilationStatus(FragmentShader));

        GLuint GeometryShader = 0;
        if (Config.GeometrySource != nullptr)
        {
            GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(GeometryShader, 1, &Config.GeometrySource, nullptr);
            glCompileShader(GeometryShader);
            assert(LogShaderCompilationStatus(GeometryShader));
        }

        Program = glCreateProgram();

        glAttachShader(Program, VertexShader);
        glAttachShader(Program, FragmentShader);
        if (GeometryShader != 0) glAttachShader(Program, GeometryShader);

        glLinkProgram(Program);
        assert(LogProgramLinkStatus());

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);
        if (GeometryShader != 0) glDeleteShader(GeometryShader);

        Initialized = true;
        return true;
    }

    ShaderProgram::~ShaderProgram()
    {
        for (auto& Pair : UniformLocations)
        {
            //i know this is unsafe, but i allocated the memory so im deleting it
            char* String = const_cast<char*>(Pair.first);
            free(String);
        }
        
        glDeleteProgram(Program);
    }

    void ShaderProgram::Use()
    {
        if (Initialized == false)
        {
            Log::Error("Initialized == false");
            return;
        }
        
        glUseProgram(Program);
    }

    GLuint ShaderProgram::GetInstance() const 
    {
        if (Initialized == false)
        {
            Log::Error("Initialized == false");
            return 0;
        }
        
        return Program;
    }

    bool ShaderProgram::LogShaderCompilationStatus(GLuint Shader)
    {
        GLint CompileStatus;
        glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

        if (!CompileStatus)
        {
            GLint LogLength = 0;

            glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);

            std::vector<char> InfoLog;
            InfoLog.resize(LogLength + 1);

            glGetShaderInfoLog(Shader, static_cast<GLsizei>(InfoLog.size()), nullptr, InfoLog.data());

            std::cerr << InfoLog.data() << "\n";
            return false;
        }

        return true;
    }

    bool ShaderProgram::LogProgramLinkStatus()
    {
        GLint LinkStatus;
        glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

        if (!LinkStatus)
        {
            GLint LogLength;
            glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);
            
            std::vector<char> InfoLog;
            InfoLog.resize(LogLength + 1);

            glGetProgramInfoLog(Program, static_cast<GLsizei>(InfoLog.size()), nullptr, InfoLog.data());

            std::cerr << InfoLog.data() << "\n";
            return false;
        }

        return true;
    }
}