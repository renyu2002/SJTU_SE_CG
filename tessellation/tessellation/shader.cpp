#include "shader.h"

const char* read_shader(const char* Path)
{
    std::string Code;
    
    std::ifstream ShaderFile;
    
    // 保证ifstream对象可以抛出异常：
   ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try
    {
        // 打开文件
        ShaderFile.open(Path);
        std::stringstream ShaderStream;
        // 读取文件的缓冲内容到数据流中
        ShaderStream << ShaderFile.rdbuf();
        
        // 关闭文件处理器
        ShaderFile.close();
        
        // 转换数据流到string
        Code = ShaderStream.str();

    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    std::string* tmp = new std::string(Code);
    return tmp->c_str();
}
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* tcs, const char* tes, const char* gs)
{
    // 1. 从文件路径中获取顶点/片段着色器

    const char* vShaderCode = read_shader(vertexPath);
    const char* fShaderCode = read_shader(fragmentPath);


    // 2. 编译着色器
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    // 着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (tcs)
    {
        const char* cShaderCode = read_shader(tcs);
        const char* eShaderCode = read_shader(tes);


        // 2. 编译着色器
        unsigned int tesc, tese;
        int success;
        char infoLog[512];

        // 顶点着色器
        tesc = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tesc, 1, &cShaderCode, NULL);
        glCompileShader(tesc);
        // 打印编译错误（如果有的话）
        glGetShaderiv(tesc, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(tesc, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::tesc::COMPILATION_FAILED\n" << infoLog << std::endl;
        };

        tese = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tese, 1, &eShaderCode, NULL);
        glCompileShader(tese);
        // 打印编译错误（如果有的话）
        glGetShaderiv(tese, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(tese, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::tese::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
        glAttachShader(ID, tese);
        glAttachShader(ID, tesc);
    }
    if (gs)
    {
        const char* gShaderCode = read_shader(gs);
        


        // 2. 编译着色器
        unsigned int geom;
        int success;
        char infoLog[512];

        // 顶点着色器
        geom = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geom, 1, &gShaderCode, NULL);
        glCompileShader(geom);
        // 打印编译错误（如果有的话）
        glGetShaderiv(geom, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geom, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::gs::COMPILATION_FAILED\n" << infoLog << std::endl;
        };

        
        glAttachShader(ID, geom);

    }

    glLinkProgram(ID);
    // 打印连接错误（如果有的话）
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}


void Shader::use()
{
    glUseProgram(ID);
}
 

        void Shader::setBool(const std::string & name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void Shader::setInt(const std::string & name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void Shader::setFloat(const std::string & name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void Shader::setVec3(const std::string& name, float x,float y,float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x,y,z);
    }
   