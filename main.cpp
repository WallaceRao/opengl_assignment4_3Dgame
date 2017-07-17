#include <stdio.h>  
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <glm/glm.hpp> 
#include <GLFW/glfw3.h>
#include <SOIL.h>	
#include "model.h"
#include "textFile.h"


#define GROUP_NO 8           // The groups' count of enermies.
#define ELEMENTS_NO 30       // The enermies' count of one group
#define ENERMY_SPEED  0.1    // Enery move along z axis with speed 0.1

#define CAM_POS_BACK 0
#define CAM_POS_LEFT 1
#define CAM_POS_RIGHT 2
#define CAM_POS_UP 3
#define CAM_POS_DOWN 4

namespace GLMAIN {
	GLuint	bgTexture;			// Storage for texture of background
	GLFWwindow* window;			// Storage for glfw window
	GLuint vao;					// Storage for vao
    GLuint  vbo[3];             // Storage for vbo
	int poleBegin;			    // Storage for the index of the flag pole start point
	GLint modelLocationLoc, mvpLoc, mvLoc, samplerLoc, baseColorLoc;  // Storage For locations in shaders
    float enemyLocations[GROUP_NO][ELEMENTS_NO][3];    // We have 8 groups of enermies.
    Model *model = NULL;       // The 3D momdel of the pokemon
    float offsetx = 0.0, offsety = 0.0; // Storage for offset value of the host, this will be used to calculate the positions of enermies.
    bool paused = false;    // A flag indicates whether the game is paused by user
 
    float hostColor[4][3] = {{0.3, 0.3, 0.3},  // Host color when host is dead
                             {0.9, 0.2, 0.2},  // Host color when there is 1 life left
                             {0.6, 0.2, 0.6},  // Host color when there are 2 lives
                             {0.2, 0.2, 0.8}}; // Host color when there are 3 lives
    int lives = 3;          // The host has three lives by default.
    int windowWidth = 1280;  // The window width, defalt value is 1280
    int windowHeight = 800;  // The window height, defalt value is 800
    int camPos = CAM_POS_BACK; // Camera position, by default it is behind the host     
}


//Initialise enermy locations
void initEnemyLocations()
{
    for(int i = 0; i < GROUP_NO; i ++)
        for (int j = 0; j < ELEMENTS_NO; j ++)
        {
            int randx = rand() % 30 - 15; 
            int randy = rand() % 30 - 15;
            GLMAIN::enemyLocations[i][j][0] = randx;  // x and y are between [-15, 15]
            GLMAIN::enemyLocations[i][j][1] = randy;
            GLMAIN::enemyLocations[i][j][2] = (i+1) * 5;  // Enermies in same group have the same z value.
        }
}



// Check if an enermy hit the host
bool collisionDetect(float enemyPos[])
{    
    float distance = enemyPos[0]*enemyPos[0] + enemyPos[1]*enemyPos[1] + enemyPos[2]*enemyPos[2];
    /* 
     * Host is always at (0,0,0), the radius of the host is about 0.55.
     * If distance between this enemy and player is shorter than 1.1, there would be a collision
     */
    if(distance < 1.1*1.1)  
        return true;
    return false;
}



// Calculate enermy locations and detect collision

void reCalcEnemyLocations(bool &collision)
{
    collision = false;
    for(int i = 0; i < GROUP_NO; i ++)
        for (int j = 0; j < ELEMENTS_NO; j ++)
        {
            /*
             * If user pressed arrow key, move all enermies to simulate the host is
             * moving rather than moving the host.
             */
            GLMAIN::enemyLocations[i][j][0] += GLMAIN::offsetx;   
            GLMAIN::enemyLocations[i][j][1] += GLMAIN::offsety;
            GLMAIN::enemyLocations[i][j][2] -= ENERMY_SPEED;
            /*
             * For enermy whose z value is smaller than -1.0, 
             * it has been out of our scope, regenerate a new one.
             */
            if(GLMAIN::enemyLocations[i][j][2] < -15.0)  
            {
                int randx = rand() % 150 - 75; 
                int randy = rand() % 150 - 75;
                GLMAIN::enemyLocations[i][j][0] = randx/5.0;
                GLMAIN::enemyLocations[i][j][1] = randy/5.0;
                GLMAIN::enemyLocations[i][j][2] = GROUP_NO * 5 - 15.0;
                
            }
            if(!collision)
                collision = collisionDetect(GLMAIN::enemyLocations[i][j]);
        }
    // reset offset.
    GLMAIN::offsetx = 0;
    GLMAIN::offsety = 0;
}



// Load image and convert to texture for background	
int LoadGLTextures()								
{
	glGenTextures(1, &GLMAIN::bgTexture);					
	glBindTexture(GL_TEXTURE_2D, GLMAIN::bgTexture);
	int width, height;
	int soilForceChannels = SOIL_LOAD_RGB;
	unsigned char *image = SOIL_load_image("sky.jpg",
		&width, &height, 0, soilForceChannels);

	glTexImage2D(GL_TEXTURE_2D, // texture target
		0, // level of detail (mipmap)
		GL_RGB, // internal data format, defines
				// number of colour components
		width, // the image width
		height, // the image height
		0, // unused parameter, must be 0
		GL_RGB, // pixel data format
		GL_UNSIGNED_BYTE,// data type used for pixels
		image); // the image data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}



// Generate view matrix
glm::mat4 getViewMatrix()
{
    // By default, camera is behind the host, looking at the host
    glm::vec3 camPos(0.0f, 0.0f, -50.0f);      // camera position, the host looking at the positive direction of z axis.
    glm::vec3 camUp0(0.0f, 1.0f, 0.0f);       // up direction
    
    if(GLMAIN::camPos == CAM_POS_LEFT) // Camera is on the left size of the host
    {
        camPos =  glm::vec3(50.0, 0.0, 0.0);
        camUp0 =  glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else if(GLMAIN::camPos == CAM_POS_RIGHT) // Camera is on the right side the host
    {
        camPos =  glm::vec3(-50.0, 0.0, 0.0);
        camUp0 =  glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else if(GLMAIN::camPos == CAM_POS_UP) // Camera is on top of the host
    {
        camPos =  glm::vec3(0.0, 70.0, 0.0);
        camUp0 =  glm::vec3(0.0f, 0.0f, 1.0f);
    }
    else if(GLMAIN::camPos == CAM_POS_DOWN) // Camera is under the host
    {
        camPos =  glm::vec3(0.0, -50.0, 0.0);
        camUp0 =  glm::vec3(0.0f, 0.0f, 1.0f);
    }
    else if(GLMAIN::camPos != CAM_POS_BACK)
    {
        printf("Error: camPosition is not correct, use the default value\n");
    }
	const glm::vec3 lookAt(0.0, 0.0, 0.0);    // point of interest
	const glm::vec3 camOffset = lookAt - camPos;
	const glm::vec3 camForward = camOffset /
		glm::length(camOffset);
	const glm::vec3 camRight = glm::cross(camForward, camUp0);
	const glm::vec3 camUp = glm::cross(camRight, camForward);

	const glm::mat4 viewRotation(
		camRight.x, camUp.x, -camForward.x, 0.f, // column 0
		camRight.y, camUp.y, -camForward.y, 0.f, // column 1
        camRight.z, camUp.z, -camForward.z, 0.f, // column 2
		0.f, 0.f, 0.f, 1.f);// column 3
	const glm::mat4 viewTranslation(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-camPos.x, -camPos.y, -camPos.z, 1);
	glm::mat4 viewMatrix = viewRotation * viewTranslation;
    return viewMatrix;
}



// Generate perspective projection matrix
void initPerspective(glm::mat4 & m)
{
    const float aspect =  GLMAIN::windowWidth * 1.0/GLMAIN::windowHeight * 1.0;
    const float zNear = 2.0f;
    const float zFar = 10.0f;
    const float zRange = zNear - zFar;
    const float tanHalfFOV = tanf(0.4);
    m[0][0] = 1.0f / (tanHalfFOV * aspect);
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f / tanHalfFOV;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = (zNear + zFar) / (zNear - zFar);
    m[2][3] = 2.0f * zFar * zNear /  (zNear - zFar);
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = -1.0f;
    m[3][3] = 0.0f;
}



// Draw the host from model "pokemon.obj"
void drawHost()
{
    float playerLocation[3] = {0,0,0};
    if(GLMAIN::modelLocationLoc != -1)
        glUniform3fv(GLMAIN::modelLocationLoc, 1, &playerLocation[0]);
            
    if(GLMAIN::baseColorLoc != -1)
        glUniform3fv(GLMAIN::baseColorLoc, 1, &GLMAIN::hostColor[GLMAIN::lives][0]);
        
    /*
     * The model is too small, so enlarge it by 20 times.
     * Rotate the model to make the host face enermies.
     */
    glm::mat4 modelMatrix = glm::mat4(20.0, 0.0f, 0.0, 0.0,
                                      0.0, 20.0f, 0.0, 0.0,
                                      0.0, 0.0f, -20.0, 0.0,
                                      0.0, 0.0f, 0.0, 1.0);
	glm::mat4 mvp = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    initPerspective(projectionMatrix);
    glm::mat4 viewMatrix = getViewMatrix();

	mvp = projectionMatrix * viewMatrix * modelMatrix;
	float mvpFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvpFloat[i * 4] = mvp[i].x;
		mvpFloat[i * 4 + 1] = mvp[i].y;
		mvpFloat[i * 4 + 2] = mvp[i].z;
		mvpFloat[i * 4 + 3] = mvp[i].w;

	}
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    float mvFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvFloat[i * 4] = modelViewMatrix[i].x;
		mvFloat[i * 4 + 1] = modelViewMatrix[i].y;
		mvFloat[i * 4 + 2] = modelViewMatrix[i].z;
		mvFloat[i * 4 + 3] = modelViewMatrix[i].w;

	}
   	 if(GLMAIN::mvLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvLoc, 1, false, mvFloat);
        
	if(GLMAIN::mvpLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvpLoc, 1, false, mvpFloat);
    
    // model->meshes contains face info of the model
    std::vector<Mesh> meshes = GLMAIN::model->meshes;  
    for (int i = 0; i < meshes.size(); i ++)
    {
        Mesh mesh = meshes[i];
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }
}



// Draw enermies.
void drawEnemies()
{
    float color[3] = {0.3, 0.8, 0.2};
    if(GLMAIN::baseColorLoc != -1)
        glUniform3fv(GLMAIN::baseColorLoc, 1, &color[0]);
    // The model is too small, so scale it by 20 times, let the heads of enermies face the camera.
    glm::mat4 modelMatrix = glm::mat4(20.0, 0.0f, 0.0, 0.0,
                                      0.0, 20.0f, 0.0, 0.0,
                                      0.0, 0.0f, 20.0, 0.0,
                                      0.0, 0.0f, 0.0, 1.0);
	glm::mat4 mvp = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    initPerspective(projectionMatrix);
    glm::mat4 viewMatrix = getViewMatrix();

	
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	float mvpFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvpFloat[i * 4] = mvp[i].x;
		mvpFloat[i * 4 + 1] = mvp[i].y;
		mvpFloat[i * 4 + 2] = mvp[i].z;
		mvpFloat[i * 4 + 3] = mvp[i].w;

	}
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    float mvFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvFloat[i * 4] = modelViewMatrix[i].x;
		mvFloat[i * 4 + 1] = modelViewMatrix[i].y;
		mvFloat[i * 4 + 2] = modelViewMatrix[i].z;
		mvFloat[i * 4 + 3] = modelViewMatrix[i].w;

	}
   	 if(GLMAIN::mvLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvLoc, 1, false, mvFloat);
        
	if(GLMAIN::mvpLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvpLoc, 1, false, mvpFloat);
        
    std::vector<Mesh> meshes =  GLMAIN::model->meshes;
    
    /*
     * Draw a mesh for all enermies, and then draw next mesh, instead of drawing the
     * whole model at one time, this could reduce the times of binding VAO, and 
     * improve performance.
     */
    
    for (int i = 0; i < meshes.size(); i ++)
    {
        Mesh mesh = meshes[i];
        glBindVertexArray(mesh.VAO);
        for(int i = 0; i < GROUP_NO; i ++)
            for(int j = 0; j < ELEMENTS_NO; j ++)
            {
                if(GLMAIN::modelLocationLoc != -1)
                    glUniform3fv(GLMAIN::modelLocationLoc, 1, &GLMAIN::enemyLocations[i][j][0]);
                glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
                
            }
    }
        
}


// Draw background, the sky.
void drawBackground()
{
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 mvp = glm::mat4(1.0f);
   
    // The camera position for background is fixed, so that the background is not changed.
    int tempCamPos = GLMAIN::camPos;
    GLMAIN::camPos = CAM_POS_BACK;
    glm::mat4 viewMatrix = getViewMatrix();
    GLMAIN::camPos = tempCamPos;
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    initPerspective(projectionMatrix);

	
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	float mvpFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvpFloat[i * 4] = mvp[i].x;
		mvpFloat[i * 4 + 1] = mvp[i].y;
		mvpFloat[i * 4 + 2] = mvp[i].z;
		mvpFloat[i * 4 + 3] = mvp[i].w;

	}
	if(GLMAIN::mvpLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvpLoc, 1, false, mvpFloat);
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    float mvFloat[16];
	for (int i = 0; i < 4; i++)
	{
		mvFloat[i * 4] = modelViewMatrix[i].x;
		mvFloat[i * 4 + 1] = modelViewMatrix[i].y;
		mvFloat[i * 4 + 2] = modelViewMatrix[i].z;
		mvFloat[i * 4 + 3] = modelViewMatrix[i].w;

	}
   	 if(GLMAIN::mvLoc != -1)
		glUniformMatrix4fv(GLMAIN::mvLoc, 1, false, mvFloat);
    glBindVertexArray(GLMAIN::vao);
    
    // THe location of background is fixed, no offset is applied.
    float modelLocation[3] = {0.0f};
    if(GLMAIN::modelLocationLoc != -1)
            glUniform3fv(GLMAIN::modelLocationLoc, 1,modelLocation);
    // Draw the surface
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GLMAIN::bgTexture);
	if (GLMAIN::samplerLoc != -1)
	{
		glUniform1i(GLMAIN::samplerLoc, 0);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
 
}


// Main render fuction
void display(void) 
{
    static int twinkleCount = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();
    
    bool collision = false;
    if(!GLMAIN::paused && GLMAIN::lives)
        reCalcEnemyLocations(collision);
    if(collision && twinkleCount == 0) // Ignore collision If host is twinkling
    {
        GLMAIN::lives--;    // Lost one life if collision happens
        if(GLMAIN::lives > 0)
            twinkleCount = 60; // twinkle in the following 60 times refreshing
    }
    /*
     * If twinkleCount %2 equals 0, then drawHost, otherwise do not 
     * draw host, so the host looks twinkling. 
     */
    if(twinkleCount > 0)
    {
        if(twinkleCount %2 == 0)
            drawHost();
        twinkleCount --;
            
    } else {
        drawHost();
    }
    drawEnemies();
    
	glfwSwapBuffers(GLMAIN::window);
    
	// Poll for and process events
	glfwPollEvents();
}



// Init vao, vbo for background
void initVAO() 
{
	float vertices[12];
    float normals[12] = {0.0f, 0.0f, 1.0f,
                         0.0f, 0.0f, 1.0f,
                         0.0f, 0.0f, 1.0f,
                         0.0f, 0.0f, 1.0f};
	float texArray[8];
	// 4 points
	vertices[0] = -1000.0f;  // bottom-left
	vertices[1] = -1000.0f;
    // The objects that behind z=200 will be covered by background.
	vertices[2] = vertices[5] = vertices[8] = vertices[11] = 200.0f;  
	vertices[3] = 1000.0f;   // bottom-right
	vertices[4] = -1000.0f;
	vertices[6] = -1000.0f;  // top-left
	vertices[7] = 1000.0f;
	vertices[9] = 1000.0f;   // top-right
	vertices[10] = 1000.0f;

	texArray[0] = 0.0;  // bottom-left
	texArray[1] = 0.0;
	texArray[2] = 1.0;
	texArray[3] = 0.0;
	texArray[4] =0.0;
	texArray[5] = 1.0;
	texArray[6] = 1.0;
	texArray[7] = 1.0;


	glGenVertexArrays(1, &GLMAIN::vao);
	glBindVertexArray(GLMAIN::vao);

	glGenBuffers(3, GLMAIN::vbo);

    // Vertex
	glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Normal
    glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    // Textrue
	glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texArray), texArray, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, GLMAIN::vbo[2]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
}



// Init and enable vertex shader and fragment shader
int setShaders() 
{
	GLint vertCompiled, fragCompiled;
	GLint linked;
	char *vs = NULL, *fs = NULL;
	GLuint VertexShaderObject = 0;
	GLuint FragmentShaderObject = 0;
	GLuint ProgramObject = glCreateProgram();

	vs = textFileRead((char *)"test.vert");
	fs = textFileRead((char *)"test.frag");
	glUseProgram(ProgramObject);

	VertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	FragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	// Load source code into shaders.
	glShaderSource(VertexShaderObject, 1, (const char **)&vs, NULL);
	glShaderSource(FragmentShaderObject, 1, (const char **)&fs, NULL);
	// Compile the  vertex shader.
	glCompileShader(VertexShaderObject);
	glGetShaderiv(VertexShaderObject, GL_COMPILE_STATUS, &vertCompiled);
	// Compile the fragment shader
	glCompileShader(FragmentShaderObject);
	glGetShaderiv(FragmentShaderObject, GL_COMPILE_STATUS, &fragCompiled);
	if (!vertCompiled || !fragCompiled)
	{
		printf("Shader compile failed, %d, %d\n",vertCompiled,fragCompiled);
		return 0;
	}
	glAttachShader(ProgramObject, VertexShaderObject);
	glAttachShader(ProgramObject, FragmentShaderObject);
	glLinkProgram(ProgramObject);
	glGetProgramiv(ProgramObject, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		// Print logs if link shaders failed.
		GLsizei len;
		glGetProgramiv(ProgramObject, GL_INFO_LOG_LENGTH, &len);
		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(ProgramObject, len, &len, log);
		printf("Shader linking failed: %s\n", log);
		delete[] log;
		return 0;
	}
	glUseProgram(ProgramObject);
	GLMAIN::mvpLoc = glGetUniformLocation(ProgramObject, "mvp");
    GLMAIN::mvLoc = glGetUniformLocation(ProgramObject, "mv");
	GLMAIN::samplerLoc = glGetUniformLocation(ProgramObject, "sampler0");
    GLMAIN::modelLocationLoc = glGetUniformLocation(ProgramObject, "location");
    GLMAIN::baseColorLoc = glGetUniformLocation(ProgramObject, "baseColor");
    
	glDeleteShader(VertexShaderObject);
	glDeleteShader(FragmentShaderObject);
	glDeleteProgram(ProgramObject);
	return 1;
}


// Read and store the model
void initModel()
{
     GLMAIN::model = new Model("Pokemon.obj");
}


// Callback when user resize window, update view port
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    GLMAIN::windowWidth = width;
    GLMAIN::windowHeight = height;
}


// reset the game
void resetGame()
{
    initEnemyLocations();
    GLMAIN::paused = false;
    GLMAIN::lives = 3;
}



// Callback for key event
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{

    float move = 0.3;  // if user press an arrow key, move enermies by 0.3.
    if(action == GLFW_PRESS || GLFW_REPEAT)
    {
        if(GLFW_REPEAT == action)
            move = 0.2; // If it is a repeat key event,  move enermies by 0.2.
        switch(key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_RIGHT:          // Move enermies left. thus it looks like host is moving right
            GLMAIN::offsetx += move;
            GLMAIN::offsetx = GLMAIN::offsetx < -4.0 ? -4.0 : GLMAIN::offsetx;   // Make sure the offset is not too large...
            break;
        case GLFW_KEY_LEFT:   // Move enermies right
            GLMAIN::offsetx -= move;
            GLMAIN::offsetx = GLMAIN::offsetx > 4.0 ? 4.0 : GLMAIN::offsetx;
            break;
        case GLFW_KEY_DOWN:  // Move enermies up
            GLMAIN::offsety += move;
            GLMAIN::offsety = GLMAIN::offsety > 4.0 ? 4.0 : GLMAIN::offsety;
            break;
        case GLFW_KEY_UP:  // Move enermies down
            GLMAIN::offsety -= move;
            GLMAIN::offsety = GLMAIN::offsety < -4.0 ? -4.0 : GLMAIN::offsety;
            break;
        case GLFW_KEY_W:  // Camera should be on top of the host
            GLMAIN::camPos = CAM_POS_UP;
            break;
        case GLFW_KEY_S:   // Camera should be under the host
            GLMAIN::camPos = CAM_POS_DOWN;
            break;
        case GLFW_KEY_A:  // Camera should be on the left side of the host
            GLMAIN::camPos = CAM_POS_LEFT;
            break;
        case GLFW_KEY_D:  // Camera should be on the right side of the host
            GLMAIN::camPos = CAM_POS_RIGHT;
            break;
        case GLFW_KEY_B:  // Camera should be behind the host, this is also the default position
            GLMAIN::camPos = CAM_POS_BACK;
            break;
        case GLFW_KEY_SPACE:
            if(action == GLFW_PRESS)   // Pause game if user presses space key.
                GLMAIN::paused = !GLMAIN::paused;
            break;
        case GLFW_KEY_R:
            if(action == GLFW_PRESS && GLMAIN::lives == 0)  // Pause game if user presses "R" key.
            resetGame();
            break;
        }
    }  
}


// Release opjects
void clearUp()
{
    glDeleteBuffers(3, &GLMAIN::vbo[0]);
    glDeleteVertexArrays(1, &GLMAIN::vao);	
    glDeleteTextures(1, &GLMAIN::bgTexture);
}

// Main entry
int main(int argc, char* argv[])
{
	// init glfw
	if (!glfwInit())
		return -1;

	// create a windowed mode window and its OpenGL context 
	GLMAIN::window = glfwCreateWindow(1280, 800, "Cross Monster", NULL, NULL);
	if (!GLMAIN::window)
	{
		glfwTerminate();
		return -1;
	}
    
	//make the window's context current 
	glfwMakeContextCurrent(GLMAIN::window);
    glfwSetFramebufferSizeCallback(GLMAIN::window, framebuffer_size_callback);
    glfwSetKeyCallback(GLMAIN::window, key_callback);

	// init glew
	glewExperimental = GL_TRUE;
	glewInit();

	setShaders();
	initVAO();
	LoadGLTextures();
    
    initModel();
    initEnemyLocations();

	static double limitFPS = 1.0 / 30.0; // limit to 30 frames per second
	double lastTime = glfwGetTime();
	double deltaTime = 0, nowTime = 0;

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT_AND_BACK);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(GLMAIN::window))
	{
		nowTime = glfwGetTime();
		deltaTime += (nowTime - lastTime) / limitFPS;
		lastTime = nowTime;

		if (deltaTime < 1.0)
			continue;
		// - Only update at 30 frames 
		while (deltaTime >= 1.0) {
			deltaTime--;
		}
        
		display(); //  Render function
	}
    clearUp();
	glfwTerminate();
	return 0;
}
