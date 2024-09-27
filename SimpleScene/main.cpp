#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// 窗口设置
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 摄像机设置
Camera camera(glm::vec3(0.0f, 0.3f, 3.3f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间设置
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 光照设置
glm::vec3 lightPos(0.0f, 0.75f, 1.65f);
glm::vec3 cubePos(0.0f, 0.3f, 2.0f);

int main()
{
    // 初始化并配置glfw
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw创建窗口
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 告诉 GLFW 捕获我们的鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad：加载所有 OpenGL 函数指针
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 配置全局 OpenGL 状态
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // 编译shader操作
    // ------------------------------------
    Shader lightingShader("lighting.vs", "lighting.fs");
    Shader lightCubeShader("lightcube.vs", "lightcube.fs");

    // 统一设置用到的坐标信息(每一行前三个数字为点的坐标，后三个为法向量)
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    //获取各个平面的数据
    // ------------------------------------------------------------------
    float CeilingVertices[36];
    std::copy(vertices + 180, vertices + 216, CeilingVertices);
    float FloorVertices[36];
    std::copy(vertices + 144, vertices + 180, FloorVertices);
    float LWallVertices[36];
    std::copy(vertices + 72, vertices + 108, LWallVertices);
    float RWallVertices[36];
    std::copy(vertices + 108, vertices + 144, RWallVertices);
    float FWallVertices[36];
    std::copy(vertices + 0, vertices + 36, FWallVertices);


    // 载入天花板的顶点信息
    // ------------------------------------------------------------------
    unsigned int VBO1, CeilingVAO;
    {
        glGenVertexArrays(1, &CeilingVAO);
        glGenBuffers(1, &VBO1);

        glBindBuffer(GL_ARRAY_BUFFER, VBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CeilingVertices), CeilingVertices, GL_STATIC_DRAW);

        glBindVertexArray(CeilingVAO);

        // 载入位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        // 载入法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    //载入地板的顶点信息
    // ------------------------------------------------------------------
    unsigned int VBO2, FloorVAO;
    {
        glGenVertexArrays(1, &FloorVAO);
        glGenBuffers(1, &VBO2);

        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(FloorVertices), FloorVertices, GL_STATIC_DRAW);

        glBindVertexArray(FloorVAO);

        // 载入位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        // 载入法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    //载入左墙的顶点信息
    // ------------------------------------------------------------------
    unsigned int VBO3, LWallVAO;
    {
        glGenVertexArrays(1, &LWallVAO);
        glGenBuffers(1, &VBO3);

        glBindBuffer(GL_ARRAY_BUFFER, VBO3);
        glBufferData(GL_ARRAY_BUFFER, sizeof(LWallVertices), LWallVertices, GL_STATIC_DRAW);

        glBindVertexArray(LWallVAO);

        // 载入位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        // 载入法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    //载入右墙的顶点信息
    // ------------------------------------------------------------------
    unsigned int VBO4, RWallVAO;
    {
        glGenVertexArrays(1, &RWallVAO);
        glGenBuffers(1, &VBO4);

        glBindBuffer(GL_ARRAY_BUFFER, VBO4);
        glBufferData(GL_ARRAY_BUFFER, sizeof(RWallVertices), RWallVertices, GL_STATIC_DRAW);

        glBindVertexArray(RWallVAO);

        // 载入位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        // 载入法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    //载入前墙的顶点信息
    // ------------------------------------------------------------------
    unsigned int VBO5, FWallVAO;
    {
        glGenVertexArrays(1, &FWallVAO);
        glGenBuffers(1, &VBO5);

        glBindBuffer(GL_ARRAY_BUFFER, VBO5);
        glBufferData(GL_ARRAY_BUFFER, sizeof(FWallVertices), FWallVertices, GL_STATIC_DRAW);

        glBindVertexArray(FWallVAO);

        // 载入位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        // 载入法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }



    // 载入方块灯的顶点信息
    unsigned int VBO6, lightCubeVAO;
    {
        glGenVertexArrays(1, &lightCubeVAO);
        glGenBuffers(1, &VBO6);

        glBindBuffer(GL_ARRAY_BUFFER, VBO6);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(lightCubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO6);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }


    // 渲染循环
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // 时间逻辑
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        // -----
        processInput(window);

        // 开始渲染
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 确保在设置 Uniforms/Drawing 对象时激活 Shader
        //---------------------------------------------------------------------
        lightingShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        //绘制天花板
        {
            //设置光照参数
            lightingShader.setVec3("objectColor", 0.5, 0.5f, 0.5f);
            lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightPos", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // view/projection 变换
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // 世界坐标变换
            model = glm::translate(model, cubePos);
            model = glm::scale(model, glm::vec3(1.0f));
            lightingShader.setMat4("model", model);

            // 渲染
            glBindVertexArray(CeilingVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制地板
        {
            //设置光照参数
            lightingShader.setVec3("objectColor", 0.5f, 0.5f, 0.5f);
            lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightPos", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // view/projection 变换
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // 世界坐标变换
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            model = glm::scale(model, glm::vec3(1.0f));
            lightingShader.setMat4("model", model);

            // 渲染
            glBindVertexArray(FloorVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制左墙
        {
            //设置光照参数
            lightingShader.setVec3("objectColor", 1.0f, 0.0f, 0.31f);
            lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightPos", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // view/projection 变换
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // 世界坐标变换
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            model = glm::scale(model, glm::vec3(1.0f));
            lightingShader.setMat4("model", model);

            // 渲染
            glBindVertexArray(LWallVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制右墙
        {
            //设置光照参数
            lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
            lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightPos", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // view/projection 变换
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // 世界坐标变换
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            model = glm::scale(model, glm::vec3(1.0f));
            lightingShader.setMat4("model", model);

            // 渲染
            glBindVertexArray(RWallVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制前墙
        {
            //设置光照参数
            lightingShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightingShader.setVec3("lightPos", lightPos);
            lightingShader.setVec3("viewPos", camera.Position);

            // view/projection 变换
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);

            // 世界坐标变换
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            model = glm::scale(model, glm::vec3(1.0f));
            lightingShader.setMat4("model", model);

            // 渲染
            glBindVertexArray(FWallVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制灯方块
        {
            lightCubeShader.use();
            lightCubeShader.setMat4("projection", projection);
            lightCubeShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
            lightCubeShader.setMat4("model", model);

            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // glfw：交换缓冲区和轮询 IO 事件（按下/释放键、移动鼠标等）
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // （可选）一旦资源超出其用途，就取消分配所有资源：
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &CeilingVAO);
    glDeleteVertexArrays(1, &FloorVAO);
    glDeleteVertexArrays(1, &RWallVAO);
    glDeleteVertexArrays(1, &LWallVAO);
    glDeleteVertexArrays(1, &FWallVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &VBO3);
    glDeleteBuffers(1, &VBO4);
    glDeleteBuffers(1, &VBO5);
    glDeleteBuffers(1, &VBO6);


    // glfw：终止，清除所有以前分配的 GLFW 资源。
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//查询 GLFW 是否按下/释放了该帧的相关键并做出相应的反应
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw：每当窗口大小发生变化（通过操作系统或用户调整大小）时，此回调函数都会执行
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 确保视区与新的窗口尺寸匹配;请注意，width和height将明显大于 Retina 显示屏上指定的高度
    glViewport(0, 0, width, height);
}


// glfw: 每当鼠标移动时，该回调都会被调用
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 反转，因为 y 坐标从下到上

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw:每当鼠标滚轮滚动时，该回调都会被调用
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}