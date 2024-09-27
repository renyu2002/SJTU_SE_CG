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

float quad[] = {
    -1,-1,
    1,-1,
    -1,1,
    1,1,
    -1,1,
    1,-1,
};
class curve {
private:
    int m_num;
public:
    float* verts;
    float* curveverts;
    int num;
    int samplenum = 100;
    void update()
    {
        if (num != m_num)
        {
            delete(verts);
            verts = new float[2 * num];
            for (int i = 0; i < num; ++i)
            {
                verts[2 * i] = -0.5 + 1.0 / (num - 1) * i;
                verts[2 * i + 1] = 0.0;
            }
            m_num = num;
        }
    }
    float getparam(int n, int x, float t)
    {
        float res = pow(t, x) * pow(1.0-t, n - x);
        long up = 1;
        long down = 1;
        for (int i = 0; i < x; ++i)
        {
            up *= (n - i);
            down *= i + 1;
        }
        return up / down * res;
    }
    void gencurve()
    {
        delete(curveverts);
        curveverts = new float[(samplenum+1) * 2];
        memset(curveverts, 0, (samplenum + 1) * 2*sizeof(float));
        for (int i = 0; i <= samplenum; ++i)
        {
            float t = i / (float)samplenum;
            for (int n = 0; n < num; ++n)
            {
                float param = getparam(num-1, n, t);
                curveverts[2 *i] = curveverts[2 * i] + verts[2 * n] * param;
                curveverts[2 * i+1] = curveverts[2 * i+1] + verts[2 * n+1] * param;
            }
        }
        
    }
    void updategl(unsigned int cvao, unsigned int cvbo, unsigned int pvao,unsigned int pvbo, unsigned int ivbo)
    {
        glBindVertexArray(cvao);
        glBindBuffer(GL_ARRAY_BUFFER, cvbo);
        glBufferData(GL_ARRAY_BUFFER, (samplenum+1) * 2 * sizeof(float), curveverts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(pvao);
        
        glBindBuffer(GL_ARRAY_BUFFER, pvbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float) * 2, quad, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, ivbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num, verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, ivbo);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1, 1);
    }

};

void gen_vao(curve* mycurve)
{

}
int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    int curcontrolpoint=0;
    curve* mycurve=new curve();
    mycurve->num = 3;
    mycurve->verts = new float[mycurve->num *2]();
    for (int i = 0; i < 3; ++i)
    {
        mycurve->verts[i * 2] = 0.5*(i-1);
        mycurve->verts[i * 2+1] = 0;        
    }


    

    unsigned int curveVAO;
    glGenVertexArrays(1, &curveVAO);
    unsigned int curveVBO;
    glGenBuffers(1, &curveVBO);

    unsigned int pointVAO;
    glGenVertexArrays(1, &pointVAO);
    unsigned int pointVBO;
    glGenBuffers(1, &pointVBO);

    

    


    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    



    Shader shader1("test.vert", "test.frag");
    Shader shader2("point.vert", "test.frag");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
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
        ImGui::InputInt("control point num", &mycurve->num);
        ImGui::SliderInt("cur control point", &curcontrolpoint, 0, mycurve->num - 1);
        ImGui::SliderFloat("controlpoint x", (mycurve->verts + 2 * curcontrolpoint), -1.0, 1.0);
        ImGui::SliderFloat("controlpoint y", (mycurve->verts + 2 * curcontrolpoint+1), -1.0, 1.0);
        ImGui::End();
        ImGui::Render(); 
        mycurve->gencurve();
        mycurve->update();
        mycurve->updategl(curveVAO,curveVBO,pointVAO,pointVBO,instanceVBO);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        processInput(window);

        
        shader1.use();
        shader1.setVec3("color", 1.0, 1.0, 1.0);
        
        glBindVertexArray(curveVAO);
        
        
        glDrawArrays(GL_LINE_STRIP, 0, mycurve->samplenum+1);
        
        
        shader2.use();
        shader2.setVec3("color", 1.0, 0.0,0.0);
        shader2.setFloat("scale", 0.01);
        glBindVertexArray(pointVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, mycurve->num);



        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
