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
void compile_shader(const char * shadercode,unsigned int &shader, GLenum shaderType)
{
    
    int success;
    char infoLog[512];

    // 顶点着色器
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shadercode, NULL);
    glCompileShader(shader);
    // 打印编译错误（如果有的话）
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
}
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* tcs, const char* tes, const char* gs)
{
    // 1. 从文件路径中获取顶点/片段着色器

    const char* vShaderCode = read_shader(vertexPath);
    const char* fShaderCode = read_shader(fragmentPath);


    // 2. 编译着色器
    unsigned int vertex, fragment;
    compile_shader(vShaderCode, vertex, GL_VERTEX_SHADER);

    compile_shader(fShaderCode, fragment, GL_FRAGMENT_SHADER);

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
        compile_shader(cShaderCode, tesc, GL_TESS_CONTROL_SHADER);
        compile_shader(eShaderCode, tese, GL_TESS_EVALUATION_SHADER);
        glAttachShader(ID, tese);
        glAttachShader(ID, tesc);
    }
    if (gs)
    {
        const char* gShaderCode = read_shader(gs);
        

        unsigned int geom;
        compile_shader(gShaderCode, geom, GL_GEOMETRY_SHADER);

        
        glAttachShader(ID, geom);

    }
    int success;
    char infoLog[512];
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
    
    ComputeShader::ComputeShader(const char* cspath)
    {
        const char* cscode = read_shader(cspath);
        unsigned int cs;
        compile_shader(cscode, cs, GL_COMPUTE_SHADER);
        ID = glCreateProgram();
        glAttachShader(ID, cs);
        glLinkProgram(ID);
        int success;
        char infoLog[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

    }