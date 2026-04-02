#pragma once
#include <glad/glad.h>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <OpenGlBase/Debug/Log.h>

namespace Base
{
    struct ShaderProgramConfig
    {
        const char* VertexSource;
        const char* FragmentSource;
        const char* GeometrySource;
    };
    
    class ShaderProgram
    {
    public:
        //preventing copying of OpenGl Handles
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        ShaderProgram() : Initialized(false) {};
        ~ShaderProgram();

        bool Init(const ShaderProgramConfig& Config);

        void Use();
        GLuint GetInstance() const;

        template<typename T>
        bool SetUniform(const char* Name, const T& Value)
        {
            if (Initialized == false)
            {
                Log::Error("Initialized == false");
                return false;
            }
            
            GLint Location = GetUniformLocation(Name);
            if (Location == -1)
                return false;
            
            SetUniform(Location, Value);

            return true;
        }

        template<typename T>
        bool SetUniform(const char* Name, const T* Values, const GLsizei Count)
        {
            if (Initialized == false)
            {
                Log::Error("Initialized == false");
                return false;
            }
            
            GLint Location = GetUniformLocation(Name);
            if (Location == -1)
                return false;
            
            SetUniform(Location, Values, Count);

            return true;
        }

    private:
        bool Initialized;

        using HashFunction = std::function<size_t(const char*)>;
        using EqualFunction = std::function<bool(const char*, const char*)>;

        HashFunction Hash = [](const char* String) -> size_t
        {
            size_t hash = 0;
            while(*String)
            {
                hash = hash * 31 + *String++;
            }
            return hash;
        };

        EqualFunction Equal = [](const char* a, const char* b) -> bool
        {
            return std::strcmp(a, b) == 0;
        };
        
        GLuint Program = 0;
        std::unordered_map<const char*, GLint, HashFunction, EqualFunction> UniformLocations{ 0, Hash, Equal };

        GLint GetUniformLocation(const char* UniformName)
        {
            assert(UniformName);
            
            auto Iterator = UniformLocations.find(UniformName);
            if (Iterator != UniformLocations.end())
            {
                return Iterator->second;
            }
            else
            {                
                GLint Location = glGetUniformLocation(Program, UniformName);
                
                std::size_t UniformNameLength = strlen(UniformName) + 1;
                void* Buffer = malloc(UniformNameLength);

                if (Buffer == nullptr)
                    return -1;

                memcpy(Buffer, UniformName, UniformNameLength);
                
                UniformLocations.emplace(static_cast<const char*>(Buffer), Location);

                return Location;
            }
        }

        template<typename T>
        void SetUniform(GLint Location, const T& Value)
        {
            if      constexpr (std::is_same_v<T, i32>)   glUniform1i(Location, Value);
            else if constexpr (std::is_same_v<T, ivec2>) glUniform2i(Location, Value.x, Value.y);
            else if constexpr (std::is_same_v<T, ivec3>) glUniform3i(Location, Value.x, Value.y, Value.z);
            else if constexpr (std::is_same_v<T, ivec4>) glUniform4i(Location, Value.x, Value.y, Value.z, Value.w);

            else if constexpr (std::is_same_v<T, u32>)   glUniform1ui(Location, Value);
            else if constexpr (std::is_same_v<T, uvec2>) glUniform2ui(Location, Value.x, Value.y);
            else if constexpr (std::is_same_v<T, uvec3>) glUniform3ui(Location, Value.x, Value.y, Value.z);
            else if constexpr (std::is_same_v<T, uvec4>) glUniform4ui(Location, Value.x, Value.y, Value.z, Value.w);

            else if constexpr (std::is_same_v<T, f32>)   glUniform1f(Location, Value);
            else if constexpr (std::is_same_v<T, vec2>) glUniform2f(Location, Value.x, Value.y);
            else if constexpr (std::is_same_v<T, vec3>) glUniform3f(Location, Value.x, Value.y, Value.z);
            else if constexpr (std::is_same_v<T, vec4>) glUniform4f(Location, Value.x, Value.y, Value.z, Value.w);

            else if constexpr (std::is_same_v<T, mat2x2>)   glUniformMatrix2fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat3x3>)   glUniformMatrix3fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat4x4>)   glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat2x3>) glUniformMatrix2x3fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat3x2>) glUniformMatrix3x2fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat2x4>) glUniformMatrix2x4fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat4x2>) glUniformMatrix4x2fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat3x4>) glUniformMatrix3x4fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else if constexpr (std::is_same_v<T, mat4x3>) glUniformMatrix4x3fv(Location, 1, GL_FALSE, glm::value_ptr(Value));
            else static_assert(sizeof(T) == 0, "Unsupported uniform type");
        }

        template <typename T>
        void SetUniform(GLint Location, const T* Values, const GLsizei Count)
        {
            if      constexpr (std::is_same_v<T, i32>)   glUniform1iv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, ivec2>) glUniform2iv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, ivec3>) glUniform3iv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, ivec4>) glUniform4iv(Location, Count, Values);

            else if constexpr (std::is_same_v<T, u32>)   glUniform1uiv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, uvec2>) glUniform2uiv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, uvec3>) glUniform3uiv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, uvec4>) glUniform4uiv(Location, Count, Values);

            else if constexpr (std::is_same_v<T, f32>)   glUniform1fv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, vec2>) glUniform2fv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, vec3>) glUniform3fv(Location, Count, Values);
            else if constexpr (std::is_same_v<T, vec4>) glUniform4fv(Location, Count, Values);

            else if constexpr (std::is_same_v<T, mat2x2>)   glUniformMatrix2fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat3x3>)   glUniformMatrix3fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat4x4>)   glUniformMatrix4fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat2x3>) glUniformMatrix2x3fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat3x2>) glUniformMatrix3x2fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat2x4>) glUniformMatrix2x4fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat4x2>) glUniformMatrix4x2fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat3x4>) glUniformMatrix3x4fv(Location, Count, GL_FALSE, Values);
            else if constexpr (std::is_same_v<T, mat4x3>) glUniformMatrix4x3fv(Location, Count, GL_FALSE, Values);
            else static_assert(sizeof(T) == 0, "Unsupported uniform type");
        }

        bool LogShaderCompilationStatus(GLuint Shader);
        bool LogProgramLinkStatus();
    };
}

