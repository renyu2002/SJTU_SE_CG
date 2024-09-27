#include <stdio.h>
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<imgui.h>
#include<imgui_impl_glfw.h>
#include<imgui_impl_opengl3.h>
#include "shader.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}





int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::cout << glGetString(GL_VERSION) << "\n";

    
    int particlenum = 10;

    float* verts = new float[2 * particlenum];
    float* vel = new float[2 * particlenum];
    for (int i = 0; i < 2*particlenum; ++i)
    {
        verts[i] = (rand() / (float)(RAND_MAX)) * 2 - 1;
        vel[i] = (rand() / (float)(RAND_MAX)) * 2 - 1;

    
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, particlenum * 2 * sizeof(float), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    unsigned int pssbo;
    glGenBuffers(1, &pssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particlenum * 4*sizeof(float), verts, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0,pssbo);

    unsigned int vssbo;
    glGenBuffers(1, &vssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particlenum * 4 * sizeof(float), vel, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vssbo);


    Shader shader1("test.vert", "test.frag");
    ComputeShader cs("test.comp");
    //Shader shader1("test.vert", "test.frag","test.tesc","test.tese","test.gs");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    int inner = 2;
    int outer = 2;
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    while (!glfwWindowShouldClose(window))
    {

        bool imguiactive = true;
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("control", &imguiactive, ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowPos(ImVec2(2, 2));
        ImGui::SetWindowSize(ImVec2(400, 200));
        ImGui::Text("particles");
        ImGui::End();
        ImGui::Render(); 
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        processInput(window);

        
        shader1.use();
        shader1.setVec3("color", 1.0, 1.0, 1.0);
        shader1.setInt("inner", inner);
        shader1.setInt("outer", outer);
        glBindVertexArray(VAO);
        
        
        glDrawArrays(GL_POINTS, 0, particlenum);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vssbo);

        glUseProgram(cs.ID);
        glUniform1f(glGetUniformLocation(cs.ID, "dt"), 0.01);
        glDispatchCompute(particlenum, 1, 1);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pssbo);
        float * data=(float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particlenum * 2 * sizeof(float), GL_MAP_READ_BIT);

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particlenum * 2 * sizeof(float), data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);





        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
