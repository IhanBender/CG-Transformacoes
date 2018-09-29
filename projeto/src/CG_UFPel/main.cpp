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
    
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    bool t1 = false, t2 = false, t3 = false, t4 = false;
    bool s1 = false, s2 = false, s3 = false, s4 = false;
    bool r1 = false, r2 = false, r3 = false, r4 = false;
    bool sh1 = false, sh2 = false, sh3 = false, sh4 = false;
    bool obj1 = false, obj2 = false, obj3 = false, obj4 = false, obj5 = false;
    bool createModel = false;
    vector<Model> models;
    char path[100];
    strcpy(path, "resources/objects/rock/rock.obj");
    bool chooseModel;
    bool chooseModelb;
    int currentModel = 0;
    
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
        // Choose .obj
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)   obj1 = true;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)   obj2 = true;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)   obj3 = true;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)   obj4 = true;
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)   obj5 = true;

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE && obj1){
            strcpy(path, "resources/objects/planet/planet.obj");
            printf("Current .obj: %s\n", path);
            obj1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE && obj2){
            strcpy(path, "resources/objects/rock/rock.obj");
            printf("Current .obj: %s\n", path);
            obj2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE && obj3){
            strcpy(path, "resources/objects/cyborg/cyborg.obj");
            printf("Current .obj: %s\n", path);
            obj3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE && obj4){
            strcpy(path, "resources/objects/nanosuit/nanosuit.obj");
            printf("Current .obj: %s\n", path);
            obj4 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE && obj5){
            strcpy(path, "resources/objects/cube/cube.obj");
            printf("Current .obj: %s\n", path);
            obj5 = false;
        }

        // Choose Model

        // Advances to next model on the list
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && models.size() > 0)   chooseModel = true;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && chooseModel) {
            chooseModel = false;

            if(currentModel >= models.size() - 1)
                currentModel = 0;
            else 
                currentModel++;
            printf("Current Model: Model number %d\n", currentModel);
        }

        // Go back to the previous model on the list
        if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && models.size() > 0)   chooseModelb = true;
        if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE && chooseModelb) {
            chooseModelb = false;
            if(currentModel <= 0)
                currentModel = models.size() - 1;
            else 
                currentModel--;
            printf("Current Model: Model number %d\n", currentModel);
        }

        // Create Model
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)   createModel = true;
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE && createModel == true) {
            // load model
            // -----------
            createModel = false;
            Model ourModel(FileSystem::getPath(path));
            models.push_back(ourModel);
        }
         
        // Shear
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && models.size() > 0)   sh1 = true;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && models.size() > 0)   sh2 = true;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && models.size() > 0)   sh3 = true;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && models.size() > 0)   sh4 = true;

        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE && sh1){
            models[currentModel].ShearX(1, 0, 2);   
            sh1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE && sh2){
            models[currentModel].ShearX(1, 1, 2);
            sh2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE && sh3){
            models[currentModel].ShearX(0, 1, 2);
            sh3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && sh4){
            models[currentModel].ShearZ(1, 0, 2);
            sh4 = false;
        }

        // Translate
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && models.size() > 0)   t1 = true;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && models.size() > 0)   t2 = true;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && models.size() > 0)   t3 = true;
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && models.size() > 0)   t4 = true;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && t1){
            models[currentModel].Translate(glm::vec3(0.5, 0.5, 0.5), 3);   
            t1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && t2){
            models[currentModel].Translate(glm::vec3(-0.5, -0.5, 0.5), 3);
            t2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && t3){
            models[currentModel].Translate(glm::vec3(0, 0, 0), 0);
            t3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE && t4){
            models[currentModel].Translate(glm::vec3(1, -1, 1), 5);
            t4 = false;
        }

        // Rotate
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && models.size() > 0)   r1 = true;
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && models.size() > 0)   r2 = true;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && models.size() > 0)   r3 = true;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && models.size() > 0)   r4 = true;

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE && r1){   
            models[currentModel].RotateAx(glm::radians(90.0), 5, glm::vec3(0, 1, 0));
            r1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && r2){
            models[currentModel].RotateAx(glm::radians(180.0), 5, glm::vec3(1, 0, 0));
            r2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE && r3){
            models[currentModel].RotateAx(glm::radians(60.0), 5, glm::vec3(0, -1, 0));
            r3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE && r4){
            models[currentModel].RotateAx(glm::radians(310.0), 2, glm::vec3(0.71, -0.45, 1.1));
            r4 = false;
        }

        // Scale
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && models.size() > 0)   s1 = true;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && models.size() > 0)   s2 = true;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && models.size() > 0)   s3 = true;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && models.size() > 0)   s4 = true;

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE && s1){
            models[currentModel].Scale(glm::vec3(0.2, 0.2, 0.2), 5);
            s1 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE && s2){
            models[currentModel].Scale(glm::vec3(0.7, 0.5, 0.25), 4);
            s2 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && s3){
            models[currentModel].Scale(glm::vec3(1, 1, 1), 0);
            s3 = false;
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE && s4){
            models[currentModel].Scale(glm::vec3(1.5, 1.5, 1.5), 2);
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


        // render the loaded models
        for(size_t i = 0; i < models.size(); i++){
            glm::mat4 model = models[i].TrasformationMatrix(currentFrame);
            ourShader.setMat4("model", model);
            models[i].Draw(ourShader);    
        }


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
