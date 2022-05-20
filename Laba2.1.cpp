#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "math_3d.h"

GLuint VBO; //инициализируем указатель на хранение вершин
GLuint gWorldLocation;//указатель для доступа к всемирной матрице в виде uniform переменной

//статические переменные для хранения вершинного и пиксельного шейдеров
static const char* pVS = "                                                          \n\
#version 330 //строка говорит, что мы хотим использовать GLSL версии 3.3            \n\
                                                                                    \n\
layout (location = 0) in vec3 Position; //строка объявляет, что вершина содержит атрибут, являющийся вектором из 3 элементов типа floats, который будет иметь имя 'Position' внутри шейдера \n\
                                                                                    \n\
uniform mat4 gWorld;//Это uniform-переменная типа матрицы 4                         \n\
                                                                                    \n\
void main() //входная точка в шейдер                                                \n\
{                                                                                   \n\
    gl_Position = gWorld * vec4(Position, 1.0);                                     \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);//задаем FlagColor красным                 \n\
}";

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;

    Scale += 0.001f;

    Matrix4f World; //матрица 4 на 4
    //ПЕРЕМЕЩЕНИЕ
    World.m[0][0] = 1.0f; World.m[0][1] = 0.0f; World.m[0][2] = 0.0f; World.m[0][3] = sinf(Scale);
    World.m[1][0] = 0.0f; World.m[1][1] = 1.0f; World.m[1][2] = 0.0f; World.m[1][3] = 0.0f;
    World.m[2][0] = 0.0f; World.m[2][1] = 0.0f; World.m[2][2] = 1.0f; World.m[2][3] = 0.0f;
    World.m[3][0] = 0.0f; World.m[3][1] = 0.0f; World.m[3][2] = 0.0f; World.m[3][3] = 1.0f;

    //ВРАЩЕНИЕ 
    World.m[0][0] = cosf(Scale); World.m[0][1] = -sinf(Scale); World.m[0][2] = 0.0f; World.m[0][3] = 0.0f;
    World.m[1][0] = sinf(Scale); World.m[1][1] = cosf(Scale);  World.m[1][2] = 0.0f; World.m[1][3] = 0.0f;
    World.m[2][0] = 0.0f;        World.m[2][1] = 0.0f;         World.m[2][2] = 1.0f; World.m[2][3] = 0.0f;
    World.m[3][0] = 0.0f;        World.m[3][1] = 0.0f;         World.m[3][2] = 0.0f; World.m[3][3] = 1.0f;

    // ПРЕОБРАЗОВАНИЕ МАСШТАБА
    World.m[0][0] = sinf(Scale); World.m[0][1] = 0.0f;        World.m[0][2] = 0.0f;        World.m[0][3] = 0.0f;
    World.m[1][0] = 0.0f;        World.m[1][1] = cosf(Scale); World.m[1][2] = 0.0f;        World.m[1][3] = 0.0f;
    World.m[2][0] = 0.0f;        World.m[2][1] = 0.0f;        World.m[2][2] = sinf(Scale); World.m[2][3] = 0.0f;
    World.m[3][0] = 0.0f;        World.m[3][1] = 0.0f;        World.m[3][2] = 0.0f;        World.m[3][3] = 1.0f;

    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);//загрузка данных в uniform переменные, загрузка матрицы

    glEnableVertexAttribArray(0);//параметры вершины
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//массив будет хранить буфер вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);//данные внутри буфера

    glDrawArrays(GL_TRIANGLES, 0, 3); //отрисовка треугольника

    glDisableVertexAttribArray(0);//отключение атрибута вершины

    glutSwapBuffers();
}


static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB); //указываем функцию рендера в качестве ленивой
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[3]; //массив из трез элементов
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);//3 вершины для треугольника

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);//приготовка к отрисовке
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);//принимает название цели
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj); //компиляция шейдера
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }//получаем статус компиляции и отображаем все ошибки шейдеров в компиляторе

    glAttachShader(ShaderProgram, ShaderObj); //присоединяем скомпилированный обьект шейдера к обьекту программы
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);//линковка шейдера
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success); //проверка программных ошибок
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);//вызов проверяет сможет ли программа запуститься с текущим состоянием конвейера
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);//вызов для использования отлинкованной программы шейдеров для конвейера 

    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");//запрашиваем позицию uniform-переменной в программном объекте
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);//окно
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 06");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    CreateVertexBuffer();

    CompileShaders();

    glutMainLoop();

    return 0;
}