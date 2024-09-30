#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//#include "filesystem.h"
#include "shader_m.h"
#include "camera.h"
#include "model.h"


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(50.0f, 80.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


std::vector<std::string> faces
{
    "Cubemap/right.jpg",
    "Cubemap/left.jpg",
    "Cubemap/top.jpg",
    "Cubemap/bottom.jpg",
    "Cubemap/front.jpg",
    "Cubemap/back.jpg"
};

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


std::vector<glm::vec3> pathPoints;
bool recordPath = false; // Toggle this with a key press
float minDistance = 1.0f; // Minimum distance to travel before recording a new point
glm::vec3 lastRecordedPoint(0.0f, 0.0f, 0.0f); // Initialize with a starting point

std::vector<glm::vec3> loadPath(const std::string& filePath) {
    std::vector<glm::vec3> path;
    std::ifstream file(filePath);
    glm::vec3 point;

    while (file >> point.x >> point.y >> point.z) {
        path.push_back(point);
    }

    return path;
}

void moveKartAlongPath(const std::vector<glm::vec3>& path, float deltaTime, glm::mat4& kartModel, size_t& currentPoint) {
    //static size_t currentPoint = 0;
    float speed = 70.0f; // Set the speed of the kart

    if (currentPoint < path.size() - 1) {
        glm::vec3 currentPos = glm::vec3(kartModel[3]); // Extract the current position from the kart's model matrix
        glm::vec3 direction = glm::normalize(path[currentPoint + 1] - currentPos); // Direction vector to the next point
        glm::vec3 nextPos = currentPos + direction * speed * deltaTime; // Calculate the next position

        // Calculate the angle to rotate the kart
        float angle = -1.0f*atan2(direction.z, direction.x);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

        // Update the model matrix
        kartModel = glm::translate(glm::mat4(1.0f), nextPos); // Move to the new position
        kartModel = kartModel * rotation; // Apply the rotation

        // Check if the kart has reached the next point
        if (glm::length(nextPos - path[currentPoint + 1]) < 0.1f) {
            currentPoint++;
        }
    }
    else
    {
        currentPoint = 0;
    }
}








int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);

    GLfloat skyboxVertices[] = {
    // Positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

    

 




#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    unsigned int cubemapTexture = loadCubemap(faces);

    size_t currentPointKart1 = 0;
    size_t currentPointKart2 = 0;

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("model_loading.vs", "model_loading.frag");
    Shader skyboxShader("skybox_shader.vs", "skybox_shader.frag");

    // load models
    // -----------
    Model ourModel("C:/Users/bogdi/OneDrive/Desktop/Karting SMG/RaceTrack/scene.gltf");
    Model ourKart("C:/Users/bogdi/OneDrive/Desktop/Karting SMG/Kart/scene.gltf");


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    std::vector<glm::vec3> path = loadPath("path.txt");
    std::vector<glm::vec3> path2= loadPath("path2.txt");

    // render the loaded model
    glm::mat4 Kart1 = glm::mat4(1.0f);
    glm::mat4 Kart2 = glm::mat4(1.0f);
    //Kart1 = glm::translate(Kart1, glm::vec3(126.043f, 0.0f, -19.5184f)); // translate it down so it's at the center of the scene
    //Kart1 = initializeKartOrientation(glm::vec3(126.043f, 0.0f, -19.5184f), path);
    
    
    
    // render loop
    
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3000.0f);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
        

        ourShader.use();

        view = camera.GetViewMatrix();

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);



        // render the loaded model
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


        // render the loaded model
        glm::mat4 Kart = glm::mat4(1.0f);
        Kart = glm::translate(Kart, glm::vec3(188.0f, 0.7f, 373.0f)); // translate it down so it's at the center of the scene
        Kart = glm::rotate(Kart, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate 180 degrees around the X-axis
        Kart = glm::rotate(Kart, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Kart = glm::scale(Kart, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", Kart);
        ourKart.Draw(ourShader);

        glm::mat4 ParkedKart = glm::mat4(1.0f);
        ParkedKart = glm::translate(ParkedKart, glm::vec3(155.0f, 0.7f, 373.0f)); // translate it down so it's at the center of the scene
        ParkedKart = glm::rotate(ParkedKart, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate 180 degrees around the X-axis
        ParkedKart = glm::rotate(ParkedKart, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ParkedKart = glm::scale(ParkedKart, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", ParkedKart);
        ourKart.Draw(ourShader);




        // Assuming you've loaded the path into a variable named 'path'
        moveKartAlongPath(path, deltaTime, Kart1, currentPointKart1);

        // Render the kart
        Kart1 = glm::rotate(Kart1, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, -1.0f)); // Rotate 180 degrees around the X-axis
        Kart1 = glm::scale(Kart1, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", Kart1);
        ourKart.Draw(ourShader);

        moveKartAlongPath(path2, deltaTime, Kart2, currentPointKart2);

        // Render the kart
        Kart2 = glm::rotate(Kart2, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, -1.0f)); // Rotate 180 degrees around the X-axis
        Kart2 = glm::scale(Kart2, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", Kart2);
        ourKart.Draw(ourShader);

        

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
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        recordPath = true;
        std::cout << "Recording path..." << std::endl;
    }
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


    



// q: why is not the skybox rendered, I disabled the depth buffer writing and it still doesn't work.
// a: the skybox is rendered, but it's behind the track. You need to render the skybox first, then render the track.




