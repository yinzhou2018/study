#pragma once
#include <glad/glad.h>

const char* vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCord1;\n"
    "layout (location = 3) in vec2 aTexCord2;\n"
    "out vec3 ourColor;\n"
    "out vec2 ourTexCord1;\n"
    "out vec2 ourTexCord2;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "   ourTexCord1 = aTexCord1;\n"
    "   ourTexCord2 = aTexCord2;\n"
    "}\0";

const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "in vec2 ourTexCord1;\n"
    "in vec2 ourTexCord2;\n"
    "uniform sampler2D texture1;\n"
    "uniform sampler2D texture2;\n"
    "void main()\n"
    "{\n"
    "if (ourTexCord2.x > 1.0 || ourTexCord2.y > 1.0) { \n"
    " FragColor = texture(texture1, ourTexCord1);\n"
    "} else {\n"
    " FragColor = texture(texture2, ourTexCord2);\n"
    "}\n"
    "}\n\0";