#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <queue>

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

struct rotation {
    float inicialAngle;
    float finalAngle;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
    glm::vec3 axis;
};

struct rotationTuple {
    float angle;
    glm::vec3 axis;
};

struct translation {
    glm::vec3 newPosition;
    glm::vec3 inicialPosition;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
};

struct scale {
    glm::vec3 scale;
    glm::vec3 inicialScale;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
};

class Model 
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // Get 

    // draws the model, and thus all its meshes
    void Draw(Shader shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
    // Translates model from current position to new Position in a certain time in seconds
    void Translate(glm::vec3 nPos, float timeTaken, float currTime){
        // If time == 0 then Translates instantly
        translation t;
        t.newPosition = nPos;
        t.inicialTime = currTime;
        t.endingTime = timeTaken + currTime;
        t.inicialPosition = this->currPosition;
        translations.push(t);

    }

    // Scales model using 3 values (x, y, z) in a certain time in seconds
    void Scale(glm::vec3 nScale, float timeTaken, float currTime){
        // If time == 0 then scales instantly
        scale s;
        s.inicialTime = currTime;
        s.endingTime = currTime + timeTaken;
        s.scale = nScale;
        s.inicialScale = this->currScale;
        scales.push(s);
    }

    // Rotates model in a certain angle, in a certain time in seconds around a specific axis
    void RotateAx(float angle, float timeTaken, float currTime, glm::vec3 axis){
        float PI = 3.14159265359;
        rotation r;
        r.inicialTime = currTime;
        r.endingTime = currTime + timeTaken;
        r.inicialAngle = currAngle;
        r.finalAngle = angle * PI / 180;
        r.axis = axis;
        rotations.push(r);
    }

    // Returns a trasformation matrix to be used on model movements and refreshes model's attributes
    glm::mat4 TrasformationMatrix(float currentTime){
        this->currTime = currentTime;

        // Scales, Translates and then rotates, acording to the first elements of the queues
        glm::mat4 transform(1.0);
        // Translate
        transform = glm::translate(transform, translateVector());
        // Rotate
        rotationTuple rt = rotationData();
        transform = glm::rotate(transform, rt.angle, rt.axis);
        // Scale
        transform = glm::scale(transform, scaleVector());

        /*
        transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, 0.0f));
        float scaleAmount = sin(glfwGetTime());
        transform = glm::scale(transform, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
        */
        return transform;
    }



private:
    // Must store current position and scale
    glm::vec3 currPosition = glm::vec3(0);
    glm::vec3 currScale = glm::vec3(1.0);
    float currAngle = 0;
    glm::vec3 axis = glm::vec3(0, 1.0, 0);
    float currTime;

    queue<translation> translations;
    queue<scale> scales;
    queue<rotation> rotations;

    glm::vec3 translateVector(){
        // if there are none translations, current position is returned
        if(!translations.empty()){
            translation t = translations.front();
            float percentage = (this->currTime - t.inicialTime) / (t.endingTime - t.inicialTime);       // Percentage of trasformation that has to be done
            // if should already had transformed, do it and pops from the queue
            if(percentage >= 1){
                translations.pop();
                this->currPosition = t.newPosition;
            }
            else {
                // otherwise, calculate how much must be translate and does it (refreshing the currPosition)
                this->currPosition.x = t.inicialPosition.x + percentage * (t.newPosition.x - t.inicialPosition.x);
                this->currPosition.y = t.inicialPosition.y + percentage * (t.newPosition.y - t.inicialPosition.y);
                this->currPosition.z = t.inicialPosition.z + percentage * (t.newPosition.z - t.inicialPosition.z);
            }
        }
        return currPosition;
    }

    rotationTuple rotationData(){
        rotationTuple rt;

        if(!rotations.empty()){
            rotation r = rotations.front();
            // Percentage of trasformation that has to be done
            this->axis = r.axis;

            float percentage = (this->currTime - r.inicialTime) / (r.endingTime - r.inicialTime);
            if(percentage >= 1){
                rotations.pop();
                this->currAngle = r.finalAngle;
            }
            else {
                this->currAngle = r.inicialAngle + percentage * (r.finalAngle - r.inicialAngle);
            }
        }

        rt.angle = currAngle;
        rt.axis = axis;
        return rt;
    }

    glm::vec3 scaleVector(){
        if(!scales.empty()){
            scale s = scales.front();
            float percentage = (this->currTime - s.inicialTime) / (s.endingTime - s.inicialTime);
            if(percentage >= 1){
                scales.pop();
                this->currScale = s.scale;
            }
            else {
                // otherwise, calculate how much must be translate and does it (refreshing the currPosition)
                this->currScale.x = s.inicialScale.x + percentage * (s.scale.x - s.inicialScale.x);
                this->currScale.y = s.inicialScale.y + percentage * (s.scale.y - s.inicialScale.y);
                this->currScale.z = s.inicialScale.z + percentage * (s.scale.z - s.inicialScale.z);
            }
        }

        return currScale;
    }

    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif
