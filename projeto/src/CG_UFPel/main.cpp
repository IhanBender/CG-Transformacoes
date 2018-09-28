#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader(FileSystem::getPath("resources/cg_ufpel.vs").c_str(), FileSystem::getPath("resources/cg_ufpel.fs").c_str());

    // load models
    // -----------
    Model ourModel(FileSystem::getPath("resources/objects/planet/planet.obj"));

    
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    bool t1 = false, t2 = false, t3 = false, t4 = false;
    bool s1 = false, s2 = false, s3 = false, s4 = false;
    bool r1 = false, r2 = false, r3 = false, r4 = false;
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

    
        // Translate
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)   t1 = true;
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)   t2 = true;
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)   t3 = true;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)   t4 = true;

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && t1){   
            ourModel.Translate(glm::vec3(0.5, 0.5, 0.5), 3);
            t1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE && t2){
            ourModel.Translate(glm::vec3(-0.5, -0.5, 0.5), 3);
            t2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE && t3){
            ourModel.Translate(glm::vec3(0, 0, 0), 0);
            t3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE && t4){
            ourModel.Translate(glm::vec3(1, -1, 1), 5);
            t4 = false;
        }

        // Rotate
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)   r1 = true;
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)   r2 = true;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)   r3 = true;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)   r4 = true;

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE && r1){   
            ourModel.RotateAx(glm::radians(90.0), 5, glm::vec3(0, 1, 0));
            r1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && r2){
            ourModel.RotateAx(glm::radians(180.0), 5, glm::vec3(1, 0, 0));
            r2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE && r3){
            ourModel.RotateAx(glm::radians(60.0), 5, glm::vec3(0, -1, 0));
            r3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE && r4){
            ourModel.RotateAx(glm::radians(310.0), 2, glm::vec3(0.71, -0.45, 1.1));
            r4 = false;
        }

        // Scale
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)   s1 = true;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)   s2 = true;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)   s3 = true;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)   s4 = true;

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE && s1){
            ourModel.Scale(glm::vec3(0.5, 0.5, 0.5), 5);
            s1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE && s2){
            ourModel.Scale(glm::vec3(0.7, 0.5, 0.25), 4);
            s2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && s3){
            ourModel.Scale(glm::vec3(1, 1, 1), 0);
            s3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE && s4){
            ourModel.Scale(glm::vec3(1.5, 1.5, 1.5), 2);
            s4 = false;
        }
            

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        // render the loaded model
        
        glm::mat4 model = ourModel.TrasformationMatrix(currentFrame);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        //model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
        //model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
