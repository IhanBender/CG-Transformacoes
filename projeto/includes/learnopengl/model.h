#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/spline.hpp>
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

struct rotationAxis {
    float inicialAngle;
    float finalAngle;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
    glm::vec3 axis;
    bool ended;
};

struct rotationRoundPoint {
    float angle;
    glm::vec3 axis;
    glm::vec3 point;
    float inicialTime;
    float endingTime;
    bool ended;
};

struct translation {
    glm::vec3 newPosition;
    glm::vec3 inicialPosition;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
    bool ended;
};

struct scale {
    glm::vec3 scale;
    glm::vec3 inicialScale;
    float inicialTime;  // Time that the trasformation began
    float endingTime;   // Time for the transformation to end
    bool ended;
};

struct shear {
    // x = 0, y = 1, x = 0
    int axis;
    float inicialFirstValue;
    float inicialSecondValue;
    float firstValue;
    float secondValue;
    float inicialTime;
    float time;
    bool ended;
};

struct spline {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
    float inicialTime;
    float time;
    bool ended;
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
        currRotating.ended = true;
        currTranslating.ended = true;
        currScaling.ended = true;
        currShearing.ended = true;
        currRPRotation.ended = true;
        currPosition = glm::vec3(0,0,0);
        currScale = glm::vec3(1,1,1);
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
    // Translates model from current position to new Position in a certain time in seconds
    void Translate(glm::vec3 nPos, float timeTaken){
        // If time == 0 then Translates instantly
        translation t;
        t.newPosition = nPos;
        t.endingTime = timeTaken;
        t.ended = false;
        translations.push(t);
    }

    // Scales model using 3 values (x, y, z) in a certain time in seconds
    void Scale(glm::vec3 nScale, float time){
        scale s;
        s.endingTime = time;
        s.scale = nScale;
        s.ended = false;
        scales.push(s);
    }

    // Rotates model in a certain angle, in a certain time in seconds around a specific axis
    void RotateAx(float angle, float timeTaken, glm::vec3 axis){
        rotationAxis r;
        r.endingTime = timeTaken;
        r.finalAngle = angle;    // Converts from degrees to radians
        r.inicialAngle = 0;
        r.axis = axis;
        rotations.push(r);
    }

    void RotatePoint(float angle, float timeTaken, glm::vec3 point){
        rotationRoundPoint r;
        r.angle = angle;
        r.axis = glm::vec3(0,1,0);
        r.ended = false;
        r.endingTime = timeTaken;
        r.point = point;
        rpRotations.push(r);
    }

    void BezierCurve(
        glm::vec3 p0,
        glm::vec3 p1,
        glm::vec3 p2,
        glm::vec3 p3,
        float time)
    {
        spline b;
        b.p0 = p0;
        b.p1 = p1;
        b.p2 = p2;
        b.p3 = p3;
        b.time = time;
        b.ended = false;
        bezierCurves.push(b);
    }

     void BSplineCurve(
        glm::vec3 p0,
        glm::vec3 p1,
        glm::vec3 p2,
        glm::vec3 p3,
        float time)
    {
        spline b;
        b.p0 = p0;
        b.p1 = p1;
        b.p2 = p2;
        b.p3 = p3;
        b.time = time;
        b.ended = false;
        bSplineCurves.push(b);
    }

    // Shears
    void ShearX(float y, float z, float time){
        shear s;
        // Gets x axis (0)
        s.axis = 0; 
        s.ended = false;
        s.time = time;
        s.firstValue = y;
        s.secondValue = z;
        //transform[1][0] = y;
        //transform[2][0] = z;
        shears.push(s);
    }

    void ShearY(float x, float z, float time){
        shear s;
        // Gets y axis (1)
        s.axis = 1; 
        s.ended = false;
        s.time = time;
        s.firstValue = z;
        s.secondValue = x;
        //transform[0][1] = 1;
        //transform[2][1] = 1;
        shears.push(s);
    }

    void ShearZ(float x, float y, float time){
        shear s;
        // Gets z axis (2)
        s.axis = 2; 
        s.ended = false;
        s.time = time;
        s.firstValue = x;
        s.secondValue = y;
        //transform[0][2] = 1;
        //transform[1][2] = 1;
        shears.push(s);
    }

    // Returns a trasformation matrix to be used on model movements and refreshes model's attributes
    glm::mat4 TrasformationMatrix(float currentTime){
        this->currTime = currentTime;
        // Scales, Translates, rotates and then shears, acording to the first elements of the queues
        currPosition = bSplinePosition();
        currPosition = bezierPosition();
        glm::mat4 transform(1.0);
        glm::mat4 rotationRP = roundPointRotationMatrix(transform);
        glm::mat4 rotation = rotationData();
        glm::mat4 translate = glm::translate(transform, translateVector());
        glm::mat4 scale = glm::scale(transform, scaleVector());
        glm::mat4 shear = shearMatrix(transform);

        return translate * shear * rotationRP * rotation * scale;
    }
    

private:
    // Postion
    glm::vec3 currPosition = glm::vec3(0);
    // Scale
    glm::vec3 currScale = glm::vec3(1.0);
    // Rotation
    glm::mat4 rotationMatrix;
    // Shear
    float shearValue1 = 0;
    float shearValue2 = 0;
    int shearAxis = 0;
    // Time
    float currTime;

    queue<translation> translations;
    queue<scale> scales;
    queue<rotationAxis> rotations;
    queue<shear> shears;
    queue<rotationRoundPoint> rpRotations;
    queue<spline> bezierCurves;
    queue<spline> bSplineCurves;

    translation currTranslating;
    scale currScaling;
    rotationAxis currRotating;
    rotationRoundPoint currRPRotation;
    spline currBezier;
    spline currBSpline;
    shear currShearing;

    /*  Functions   */
    glm::vec3 translateVector(){
        if(currTranslating.ended){
            if(!translations.empty()){
                currTranslating = translations.front();
                currTranslating.ended = false;
                currTranslating.inicialPosition = currPosition;
                currTranslating.inicialTime = currTime;
                currTranslating.endingTime += currTime;
                translations.pop(); 
            }
            else {
                return currPosition;
            }
        }
        // if reached this line, has a currTranslation going on
        // Percentage of trasformation that has to be done
        translation t = currTranslating;
        float percentage = (this->currTime - t.inicialTime) / (t.endingTime - t.inicialTime);
        // if should already had transformed, do it and pops from the queue
        if(percentage >= 1){
            currTranslating.ended = true;
            this->currPosition = t.newPosition;
        }
        else {
            // otherwise, calculate how much must be translate and does it (refreshing the currPosition)
            this->currPosition.x = t.inicialPosition.x + percentage * (t.newPosition.x - t.inicialPosition.x);
            this->currPosition.y = t.inicialPosition.y + percentage * (t.newPosition.y - t.inicialPosition.y);
            this->currPosition.z = t.inicialPosition.z + percentage * (t.newPosition.z - t.inicialPosition.z);
        }
        return currPosition;
    }

    // Shear
    glm::mat4 shearMatrix(glm::mat4 transform) {
       
        if(currShearing.ended){
            if(!shears.empty()){
                currShearing = shears.front();
                currShearing.ended = false;
                currShearing.inicialTime = currTime;
                currShearing.time += currTime;
                currShearing.inicialFirstValue = 0;
                currShearing.inicialSecondValue = 0;
                // Resets shear status
                shearAxis = currShearing.axis;
                shearValue1 = 0;
                shearValue2 = 0;
                // Pops from queue
                shears.pop();
            }
            else {
                glm::mat4 sMatrix(1);
                sMatrix[(shearAxis + 1) % 3][shearAxis] = this->shearValue1;
                sMatrix[(shearAxis + 2) % 3][shearAxis] = this->shearValue2;
                return sMatrix * transform;
            }
        }

        shear s = currShearing;
        float percentage = (this->currTime - s.inicialTime) / (s.time - s.inicialTime);
        if(percentage >= 1){
            currShearing.ended = true;
            this->shearValue1 = s.firstValue;
            this->shearValue2 = s.secondValue;
        }
        else {
            this->shearValue1 = s.inicialFirstValue + percentage * (s.firstValue - s.inicialFirstValue);
            this->shearValue2 = s.inicialSecondValue + percentage * (s.secondValue - s.inicialSecondValue);
        }

        glm::mat4 sMatrix(1);
        sMatrix[(shearAxis + 1) % 3][shearAxis] = this->shearValue1;
        sMatrix[(shearAxis + 2) % 3][shearAxis] = this->shearValue2;
        return sMatrix * transform;

    }

    glm::mat4 rotationData(){

        if(currRotating.ended){

            if(!rotations.empty()){
                currRotating = rotations.front();
                currRotating.ended = false;
                currRotating.inicialAngle = 0;                
                currRotating.inicialTime = currTime;
                currRotating.endingTime += currTime;

                rotations.pop(); 
            }
            else {
                return rotationMatrix;
            }
        }

        float currAngle;
        rotationAxis r = currRotating;
        float percentage = (this->currTime - r.inicialTime) / (r.endingTime - r.inicialTime);
        if(percentage >= 1){
            currRotating.ended = true;
            rotationMatrix = glm::rotate(rotationMatrix, r.finalAngle, currRotating.axis);
            return rotationMatrix;
        }
        else {
            currAngle = r.inicialAngle + percentage * (r.finalAngle - r.inicialAngle);
        }

        glm::mat4 rMatrix = glm::rotate(rotationMatrix, currAngle, currRotating.axis);
        return rMatrix;
    }

    glm::vec3 scaleVector(){
        if(currScaling.ended){
            if(!scales.empty()){
                currScaling = scales.front();
                currScaling.ended = false;
                currScaling.inicialScale = currScale;
                currScaling.inicialTime = currTime;
                currScaling.endingTime += currTime;
                scales.pop(); 
            }
            else {
                return currScale;
            }
        }

        scale s = currScaling;
        float percentage = (this->currTime - s.inicialTime) / (s.endingTime - s.inicialTime);
        if(percentage >= 1){
            currScaling.ended = true;
            this->currScale = s.scale;
        }
        else {
            this->currScale.x = s.inicialScale.x + percentage * (s.scale.x - s.inicialScale.x);
            this->currScale.y = s.inicialScale.y + percentage * (s.scale.y - s.inicialScale.y);    
            this->currScale.z = s.inicialScale.z + percentage * (s.scale.z - s.inicialScale.z);
        }
        return currScale;
    }

    glm::mat4 roundPointRotationMatrix(glm::mat4 transform){
        glm::mat4 rMatrix = transform;

        if(currRPRotation.ended){
            if(!rpRotations.empty()){
                currRPRotation = rpRotations.front();
                currRPRotation.inicialTime = currTime;
                currRPRotation.endingTime += currTime;
                currRPRotation.ended = false;
                this->currPosition.y = currRPRotation.point.y;
                rpRotations.pop();
            }
            else {
                return glm::mat4(1.0) * transform;
            }
        }
        
        rotationRoundPoint r = currRPRotation;
        float currAngle = r.angle;
        float percentage = (this->currTime - r.inicialTime) / (r.endingTime - r.inicialTime);
        if(percentage >= 1){
            currRPRotation.ended = true;
            rMatrix = glm::translate(rMatrix, r.point);
            rMatrix = glm::rotate(rMatrix, currAngle, r.axis);
            rMatrix = glm::translate(rMatrix, -(r.point));
            
            rotationMatrix = rotationMatrix * rMatrix;
            return rMatrix;
        }
        else {
            currAngle = r.angle + percentage * (r.angle);
            rMatrix = glm::translate(rMatrix, r.point);
            rMatrix = glm::rotate(rMatrix, currAngle, r.axis);
            rMatrix = glm::translate(rMatrix, -(r.point));
            return rMatrix;
        }
        
    }

    // 
    float B0(float u){
        return  float(pow(u - 1, 3) / 6.0);
    }

    float B1(float u){
        return float((3 * pow(u, 3) - 6 * pow(u, 2) + 4) / 6.0);
    }

    float B2(float u){
        return float((-3 * pow(u, 3) + 3 * pow(u, 2) + 3 * u + 1) / 6.0);
    }

    float B3(float u){
        return float(pow(u, 3) / 6.0);
    }

    glm::vec3 BSpline(spline b, float u){
       return B0(u) * b.p0 + B1(u) * b.p1 + B2(u) * b.p2 + B3(u) * b.p3;
    }

    glm::vec3 bSplinePosition(){
        if(currBSpline.ended){
            if(!bSplineCurves.empty()){
                currBSpline = bSplineCurves.front();
                currBSpline.inicialTime = currTime;
                currBSpline.time += currTime;
                currBSpline.ended = false;
                bSplineCurves.pop();
            }
            else {
                return currPosition;
            }
        }
        
        spline b = currBSpline;
        float percentage = (this->currTime - b.inicialTime) / (b.time - b.inicialTime);
        if(percentage >= 1){
            currBSpline.ended = true;
        }
    
        glm::vec3 v = BSpline(currBSpline, percentage);
        //printf("%f %f %f\n", v.x, v.y, v.z);
        return v;
    }


    // Cubic Bézier curve Equation
    glm::vec3 Bezier(spline b, float t){
        return (float)pow(1-t, 3) * b.p0 + 
               3 * (float)pow(1-t, 2) * t * b.p1 +
               3 * (1-t) * (float)pow(t, 2) * b.p2 +
               (float)pow(t, 3) * b.p3;
    }

    glm::vec3 bezierPosition(){

        if(currBezier.ended){
            if(!bezierCurves.empty()){
                currBezier = bezierCurves.front();
                currBezier.inicialTime = currTime;
                currBezier.time += currTime;
                currBezier.ended = false;
                bezierCurves.pop();
            }
            else {
                return currPosition;
            }
        }
        
        spline b = currBezier;
        float percentage = (this->currTime - b.inicialTime) / (b.time - b.inicialTime);
        if(percentage >= 1){
            currBezier.ended = true;
            return currBezier.p3;
        }
        else {
            glm::vec3 v = Bezier(currBezier, percentage);
            //printf("%f %f %f\n", v.x, v.y, v.z);
            return v;
        }
    }


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
