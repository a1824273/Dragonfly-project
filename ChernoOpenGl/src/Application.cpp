#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

//__ means it is MSVC specific and cannot work on other compilers like gcc
#define ASSERT(x) if (!(x)) __debugbreak();

#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
    

//clears the error queue
static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

//checking and printing error
static bool GLLogCall(const char *function, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL error] (" << error << "): " << function << " " << file << ": " << line << std::endl;
        return false;
    }
    return true;
}
struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};
static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                //set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                //set mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << "\n";
        }

    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader( unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(sizeof(char) * length);
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}


static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    //vs and fs are alreadly linked and not needed anymore
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //syncing with the monitor refresh rate
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "failed to init glew" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] =
    {
        -0.5f, -0.5f,   //0
         0.5f, -0.5f,  //1
         0.5f,  0.5f,    //2
        -0.5f,  0.5f    //3
    };

    //created an index buffer
    unsigned int indices[] = {
        0,1,2,
        2,3,0
    };

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));     //binidn vertex array

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    //select the buffer, determining that buffer is an array buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    //specify the buffer
    GLCall(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), positions, GL_STATIC_DRAW));

    //determining the layout of the buffer
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));  //index 0 of the vertex array is bound tot he currentlybound GL_ARRAY_BUFFER

    //creating an index buffer object
    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    //select the buffer, determining that buffer is an element array buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    //specify the buffer
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    //gl shader declaration
    ShaderProgramSource source = ParseShader("./src/res/shaders/Basic.Shader");


    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));       //bind the shader


    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1); //  checking that the unifomrm is found or unused
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

    //unbinding everything
    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    float r = 0.0f;
    float increment = 0.05f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));


        GLCall(glUseProgram(shader));
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        
        GLCall(glBindVertexArray(vao));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

        //using the indices index buffer to draw a rectangle by creating 6 points accoridng ot the indices buffer
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
        {
            increment = -0.05f;
        }
        else if (r < -1.0f)
        {
            increment = 0.05f;
        }

        r += increment;
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}