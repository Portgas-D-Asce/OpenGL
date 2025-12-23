//
// Created by pk on 2025/12/23.
//

#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"


class Shader {
public:
    Shader(const std::string& vertex_path, const std::string& fragment_path)
        : _vertex_path(vertex_path), _fragment_path(fragment_path), _id(0) {}
    ~Shader() { glDeleteProgram(_id); }

    bool link_program() {
        const auto vertex_shader = compile_sharder(_vertex_path, GL_VERTEX_SHADER);
        std::cout << vertex_shader << std::endl;
        const auto fragment_shader = compile_sharder(_fragment_path, GL_FRAGMENT_SHADER);
        std::cout << fragment_shader << std::endl;
        _id = glCreateProgram();
        glAttachShader(_id, vertex_shader);
        glAttachShader(_id, fragment_shader);
        glLinkProgram(_id);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        // check for linking errors
        int success;
        char infoLog[512];
        glGetProgramiv(_id, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(_id, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            return false;
        }
        return true;
    }

    GLuint id() const { return _id; }

    void use() const { glUseProgram(_id); }

    static void unuse() { glUseProgram(0); }

    // 设置 float uniform
    void set_float(const std::string& name, const float value) const {
        glUniform1f(get_uniform_location(name), value);
    }

    // 设置 int uniform
    void set_int(const std::string& name, const int value) const {
        glUniform1i(get_uniform_location(name), value);
    }

    // 设置 bool uniform (实际存储为 int)
    void set_bool(const std::string& name, const bool value) const {
        glUniform1i(get_uniform_location(name), static_cast<int>(value));
    }

    // 设置 vec2 uniform
    void set_vec2(const std::string& name, const float x, float y) const {
        glUniform2f(get_uniform_location(name), x, y);
    }

    void set_vec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(get_uniform_location(name), 1, &value[0]);
    }

    // 设置 vec3 uniform
    void set_vec3(const std::string& name, const float x, const float y, const float z) const {
        glUniform3f(get_uniform_location(name), x, y, z);
    }

    void set_vec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(get_uniform_location(name), 1, &value[0]);
    }

    // 设置 vec4 uniform
    void set_vec4(const std::string& name, const float x, const float y, const float z, const float w) const {
        glUniform4f(get_uniform_location(name), x, y, z, w);
    }

    void set_vec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(get_uniform_location(name), 1, &value[0]);
    }

    // 设置 mat3 uniform
    void set_mat3(const std::string& name, const glm::mat3& mat, const bool transpose = false) const {
        glUniformMatrix3fv(get_uniform_location(name), 1, transpose ? GL_TRUE : GL_FALSE, &mat[0][0]);
    }

    // 设置 mat4 uniform
    void set_mat4(const std::string& name, const glm::mat4& mat, const bool transpose = false) const {
        glUniformMatrix4fv(get_uniform_location(name), 1, transpose ? GL_TRUE : GL_FALSE, &mat[0][0]);
    }

    // 设置纹理采样器 uniform
    void set_texture(const std::string& name, const GLuint texture_id, const int texture_unit) const {
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        set_int(name, texture_unit);
    }

    static GLuint generate_mipmap(const std::string& image_path) {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        // stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(image_path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
            if (image_path.find(".png") != std::string::npos) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
            return 0;
        }
        stbi_image_free(data);
        return texture;
    }

private:
    static std::string load_file(const std::string& path) {
        std::ifstream ifs;
        ifs.open(path);
        if (!ifs.is_open()) {
            std::cout << "Failed to open file " << path << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        return buffer.str();
    }

    static GLuint compile_sharder(const std::string& path, const GLuint type) {
        auto source = load_file(path);
        char* ptr = source.data();
        // std::cout << vertex_source << std::endl;
        const auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &ptr, nullptr);
        glCompileShader(shader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLint get_uniform_location(const std::string& name) const {
        return glGetUniformLocation(_id, name.c_str());
    }
private:
    std::string _vertex_path;
    std::string _fragment_path;
    GLuint _id;
};

#endif //SHADER_H
