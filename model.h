#pragma once

#include <GL/glew.h>
#include "mesh.h"
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// Class to load 3D model, obj format.
class Model
{
public:
	Model(GLchar* path)
	{
		this->loadModel(path);
	}
    std::vector<Mesh> meshes;
private:
	std::string directory;
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);


};

