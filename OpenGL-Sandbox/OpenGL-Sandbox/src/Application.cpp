#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureManager.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

// Settings 
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;

// Camera system
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

glm::vec2 lastMousePos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
bool firstMouse = true; // First time the mouse is captured by the window on focus

// Time step
float deltaTime = 0.0f; // Time betwwen current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// User input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void process_input(GLFWwindow* window, float ts);
int read_object_id(unsigned int id, int x, int y);

unsigned int hoveringFBO; // Framebuffer object for hovering effect
int currentEntityID = -1; // Current entity ID for debugging purposes

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Sandbox", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Capture mouse by default when the application gets focus
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set window callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[ERROR]: Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // Create shader
    Shader litShader("resources/shaders/Vertex.glsl", "resources/shaders/LitFragment.glsl");
    Shader unlitShader("resources/shaders/Vertex.glsl", "resources/shaders/UnlitFragment.glsl");
	Shader hoveringShader("resources/shaders/Vertex.glsl", "resources/shaders/HoveringFragment.glsl");

    // Create model
	//AssetLoader::Model backpackModel("resources/models/backpack/backpack.obj");

    // Load container texture
    auto orangeWallTexture = TextureManager::Instance().Get("resources/textures/proto_wall_orange.png");
    auto lightGroundTexture = TextureManager::Instance().Get("resources/textures/proto_ground_light.png");

	//Texture containerTexture("resources/textures/container2.png");
	//Texture specularTexture("resources/textures/container2_specular.png");

    // Renderer data - the vertices below define a cube that is located at the center of the screen
    float cubeVertices[] =
    {   // positions            // normals              // texture coords
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     0.0f, 0.0f,
                                 
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f,      0.0f, 0.0f,
                                 
        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
                                 
         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
                                 
        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     0.0f, 1.0f,
                                 
        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     0.0f, 1.0f
    };

    // Cubes positions
    glm::vec3 cubePositions[] =
    {
        glm::vec3(-1.0f, 0.0f, -1.0f),
        glm::vec3(2.0f, 0.0f, 0.0f)
    };

    // Plane vertices
    float planeVertices[] =
    {
        // positions            // normals          // texture coordinates (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,   2.0f, 2.0f
    };

    // Point lights positions in the world
    glm::vec3 pointLightPositions[] =
    {
        //glm::vec3(0.7f, 0.2f, 2.0f),
        //glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -2.5f),
        //glm::vec3(0.0f, 0.0f, -3.0f)
	};

    std::unique_ptr<AssetLoader::Mesh> cubeMesh = std::make_unique<AssetLoader::Mesh>(cubeVertices, sizeof(cubeVertices) / sizeof(cubeVertices[0]), 8);
    std::unique_ptr<AssetLoader::Mesh> planeMesh = std::make_unique<AssetLoader::Mesh>(planeVertices, sizeof(planeVertices) / sizeof(planeVertices[0]), 8);

	// Create the hovering effect framebuffer object (FBO)
    unsigned int hoveringTexture;
	glGenFramebuffers(1, &hoveringFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hoveringFBO);

	glGenTextures(1, &hoveringTexture);
	glBindTexture(GL_TEXTURE_2D, hoveringTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED_INTEGER, GL_INT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hoveringTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the hovering framebuffer

    // Bind the shader once, it is the same here
    litShader.Use();

    litShader.SetUniformInt("u_Material.texture_diffuse1", 0);
    litShader.SetUniformFloat("u_Material.shininess", 32.0f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle user input
        process_input(window, deltaTime);

        // Rendering anything happens here
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Wireframe mode
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        litShader.Use();

        litShader.SetVector3f("u_ViewPosition", camera.GetWorldPosition());

        // Lighting uniforms
        
        // Update the directional light uniforms
        litShader.SetUniform3f("u_DirectionalLight.direction", -0.2f, -1.0f, -0.3f); // Directional light pointing downwards
        litShader.SetUniform4f("u_DirectionalLight.ambient", 0.2f, 0.2f, 0.2f, 1.0f);
        litShader.SetUniform4f("u_DirectionalLight.diffuse", 0.8f, 0.8f, 0.8f, 1.0f);
        litShader.SetUniform4f("u_DirectionalLight.specular", 0.5f, 0.5f, 0.5f, 1.0f);

		// Update the point light uniforms
        const glm::vec3 pointLightAttenuationFactors{ 1.0f, 0.09f, 0.032f }; // Constant, linear and quadratic attenuation factors
        for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        {
            std::string pointLightName;
			pointLightName.reserve(48); // Reserve space for the string to avoid reallocations
			pointLightName = "u_PointLights[" + std::to_string(i) + "]";
            litShader.SetVector3f(pointLightName + ".position", { glm::sin(currentFrame) * pointLightPositions[i].x, pointLightPositions[i].y, glm::cos(currentFrame) * pointLightPositions[i].z });
			litShader.SetUniform4f(pointLightName + ".ambient", 0.05f, 0.05f, 0.05f, 1.0f); // Ambient light color
			litShader.SetUniform4f(pointLightName + ".diffuse", 0.8f, 0.8f, 0.8f, 1.0f); // Diffuse light color
			litShader.SetUniform4f(pointLightName + ".specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color
        
		    // We want the point light to cover a distance of 50 units, so we set the attenuation factors accordingly
		    litShader.SetUniformFloat(pointLightName + ".constant", pointLightAttenuationFactors.x);
		    litShader.SetUniformFloat(pointLightName + ".linear", pointLightAttenuationFactors.y);
		    litShader.SetUniformFloat(pointLightName + ".quadratic", pointLightAttenuationFactors.z);
        }

		// Update the spot light uniforms
		// The spot light is the camera itself, so we set its position to the camera's world position
		litShader.SetVector3f("u_SpotLight.position", camera.GetWorldPosition()); // Position of the spot light
		litShader.SetVector3f("u_SpotLight.direction", camera.GetForwardDirection()); // Direction of the spot light
		litShader.SetUniform4f("u_SpotLight.ambient", 0.0f, 0.0f, 0.0f, 1.0f); // Ambient light color
		litShader.SetUniform4f("u_SpotLight.diffuse", 1.0f, 1.0f, 1.0f, 1.0f); // Diffuse light color
		litShader.SetUniform4f("u_SpotLight.specular", 1.0f, 1.0f, 1.0f, 1.0f); // Specular light color
        litShader.SetUniformFloat("u_SpotLight.constant", pointLightAttenuationFactors.x);
        litShader.SetUniformFloat("u_SpotLight.linear", pointLightAttenuationFactors.y);
        litShader.SetUniformFloat("u_SpotLight.quadratic", pointLightAttenuationFactors.z);
		litShader.SetUniformFloat("u_SpotLight.cutOff", glm::cos(glm::radians(5.0f))); // Inner cut-off angle for the spot light
		litShader.SetUniformFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(17.5f))); // Outer cut-off angle for the spot light

        // Geometry uniforms

        // Set the model, view and projection matrix uniforms
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFOV()), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        litShader.SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
        litShader.SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

		// Make sure we don't update the stencil buffer when rendering the plane
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // Set the stencil operation to replace the stencil value when rendering the plane
		glStencilMask(0x00); // Disable writing to the stencil buffer

        lightGroundTexture->Bind();

        glm::mat4 planeModel = glm::mat4(1.0f);
        litShader.SetMatrix4f("u_Model", planeModel);

        //glDrawArrays(GL_TRIANGLES, 0, 6);
		planeMesh->Draw(litShader);

		// Enable writing to the stencil buffer again
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always pass the stencil test and set the stencil value to 1
		glStencilMask(0xFF); // Enable writing to the stencil buffer

        orangeWallTexture->Bind();

        for (auto cubePosition : cubePositions)
        {
			litShader.Use();

            // Calculate the model matrix for each cube
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePosition);
            litShader.SetMatrix4f("u_Model", cubeModel);

            // Render one cube
			cubeMesh->Draw(litShader);
        }

		// Render the hovering effect
		glBindFramebuffer(GL_FRAMEBUFFER, hoveringFBO); // Bind the hovering framebuffer
		glClear(GL_COLOR_BUFFER_BIT); // Clear the hovering framebuffer

		hoveringShader.Use();

        hoveringShader.SetMatrix4f("u_Projection", projection); // Send the projection matrix to the shader
        hoveringShader.SetMatrix4f("u_View", view); // Pass the camera view matrix to the shader

        for (auto cubePosition : cubePositions)
        {
            // Calculate the model matrix for each cube
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePosition);
            hoveringShader.SetMatrix4f("u_Model", cubeModel); // Set the model matrix for the shader

			// Render one cube
            cubeMesh->Draw(hoveringShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the hovering framebuffer

		// Render the outline of the pointed at cube if the current entity ID is the cube's one
        if (currentEntityID == cubeMesh->GetId())
        {
		    // Render upscaled cubes using the stencil buffer
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // Pass the stencil test if the stencil value is not equal to 1
            glStencilMask(0x00); // Disable writing to the stencil buffer
		    glDisable(GL_DEPTH_TEST); // Disable depth testing to render the upscaled cubes on top of the floor

		    unlitShader.Use();

            glm::mat4 upscaledCubesProjection = projection;
            glm::mat4 upscaledCubesView = view;
            unlitShader.SetMatrix4f("u_Projection", upscaledCubesProjection); // Send the projection matrix to the shader
            unlitShader.SetMatrix4f("u_View", upscaledCubesView); // Pass the camera view matrix to the shader

            // Render the cubes again, but this time they will be upscaled
            for (auto cubePosition : cubePositions)
            {
                // Calculate the model matrix for each cube
                glm::mat4 cubeModel = glm::mat4(1.0f);
                cubeModel = glm::translate(cubeModel, cubePosition);
                cubeModel = glm::scale(cubeModel, glm::vec3(1.075f)); // Scale up the cubes
                unlitShader.SetMatrix4f("u_Model", cubeModel);
                unlitShader.SetUniform4f("u_Color", 0.04f, 0.28f, 0.26f, 1.0f);

                // Render one cube
			    cubeMesh->Draw(unlitShader);
		    }
        }

        glEnable(GL_DEPTH_TEST); // Enable depth testing again
        glStencilMask(0xFF); // Enable writing to the stencil buffer again
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // Always pass the stencil test and set the stencil value to 1

		// Render the light sources
		unlitShader.Use();
        
        glm::mat4 lightProjection = projection;
        glm::mat4 lightView = view;
        unlitShader.SetMatrix4f("u_Projection", lightProjection); // Send the projection matrix to the shader
        unlitShader.SetMatrix4f("u_View", lightView); // Pass the camera view matrix to the shader
        
		// Calculate the point lights model matrices and render them
        for (auto & pointLightPosition : pointLightPositions)
        {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, { glm::sin(currentFrame) * pointLightPosition.x, pointLightPosition.y, glm::cos(currentFrame) * pointLightPosition.z });
            lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale down the light source
        
            unlitShader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
            unlitShader.SetMatrix4f("u_Model", lightModel);
        
            // Render the light source model
			cubeMesh->Draw(unlitShader);
		}

        glBindVertexArray(0);

        double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
        // Read a pixel from the framebuffer (for debugging purposes)
        if (mouseX >= 0 && mouseY >= 0 && mouseX < SCREEN_WIDTH && mouseY < SCREEN_HEIGHT)
        {
            currentEntityID = read_object_id(hoveringFBO, (int)mouseX, (int)mouseY);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glDeleteBuffers(1, &hoveringFBO);

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastMousePos = { (float)xPos, (float)yPos };
        firstMouse = false;
    }

    glm::vec2 offset = 
    { 
        xPos - lastMousePos.x, 
        lastMousePos.y - yPos // reversed since y-coordinates range from bottom to top
    };
    lastMousePos = { (float)xPos, (float)yPos };

    camera.OnMouseMove(offset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.OnMouseScroll(yOffset);
}

void process_input(GLFWwindow* window, float ts)
{
    const float cameraSpeed = 2.5f * ts;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.OnKeyPressed(ts, CameraMovement::RIGHT);
}

int read_object_id(unsigned int id, int x, int y)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    unsigned int vertexId; // RGBA format
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &vertexId);
	std::cout << "Read pixel at (" << x << ", " << y << ") from framebuffer ID: " << id
        << "Entity ID: " << (int)vertexId << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the framebuffer

	return vertexId; // Return the alpha value for further processing if needed
}
