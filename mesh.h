#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>


struct Vertex
{
	glm::vec3 Position;
    glm::vec3 Normal;
};


// Class to store mesh info of the model

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);
    void setupVAO();
    GLuint VAO, VBO, EBO;
	
};

