#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "initializer.h"
#include <map>
#include <algorithm>



#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

//PARAMATERS
GLfloat distanceFromCam = 15;


GLuint gProgram[2];
int gWidth, gHeight;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

SceneObject cube;
SceneObject tetrahedron;
SceneObject octahedron;

std::vector<SceneObject> objects;
std::vector<SceneObject> initialObjects;

vector<vector<Model>> oldModels;
int level = 0;


bool areAnimsActive = true;
VisualizationMode currentMode = VisualizationMode::Solid;

GLint gInVertexLoc, gInNormalLoc;



std::vector<float> cVA(const glm::vec3& vec3) {
    vector<float>arr;
    arr.push_back(vec3.x);
    arr.push_back(vec3.y);
    arr.push_back(vec3.z);

    return arr;
}

glm::vec3 cAV(vector<float> v)
{
    glm::vec3 g;
    g.x= v[0];
    g.y= v[1];
    g.z = v[2];
    return g;
}

void printVectorFloat(vector<vector<float>> array)
{
    for(auto elem: array)
    {
        cout<<elem[0]<<" : "<<elem[1]<<" : "<<elem[2]<<endl;
    }
}

bool containsVertex(vector<Vertex> vertexArray, Vertex vertex)
{
    for(Vertex vert: vertexArray)
    {
        if(vert.x == vertex.x && vert.y == vertex.y && vert.z == vertex.z)
            return true;
    }

    return false;
}

void subDivision(Model& model, bool isCubic)
{
    Model subdividedModel;

    map<int, vector<vector<float>>> pointFaceMapping;
    map<int, vector<vector<float>>> pointVerticeMapping;

    map<int, vector<vector<float>>> pointEdgeMapping;
    map<int, vector<vector<float>>> pointMidEdgeMapping;

    map<vector<float>, vector<int>> edgePointMapping; 
    
    
    map<vector<float>, vector<vector<float>>> edgeToFacePointMapping;
    map<vector<float>, vector<vector<float>>> faceToEdgePointMapping;

    map<vector<float>, int> indexesOfVertices;
    
    vector<vector<float>> edgePoints;

    map<vector<float>, pair<int, vector<float>>> face_edge_ids;


    int segmentFault = 0;

    cout<<"MODEL FACE_3 COUNT => "<<model.gFaces_3.size()<<endl;
    cout<<"MODEL FACE_4 COUNT => "<<model.gFaces_4.size()<<endl;
    cout<<"MODEL VERTEX COUNT => "<<model.gVertices.size()<<endl;
    
    if(isCubic)
    {
        //FOR CUBICS
 
        for (auto face : model.gFaces_4) {
            glm::vec3 facePoint(0.0f);
            for (int i = 0; i < 4 ; ++i) {
                int vertexIndex = face.vIndex[i];
                facePoint += glm::vec3(model.gVertices[vertexIndex].x, 
                model.gVertices[vertexIndex].y, model.gVertices[vertexIndex].z);
            }
            facePoint /= 4.0f;
            subdividedModel.gVertices.push_back({ facePoint.x, facePoint.y, facePoint.z });
            indexesOfVertices[cVA(facePoint)] = subdividedModel.gVertices.size()-1;
            

            pointFaceMapping[face.vIndex[0]].push_back(cVA(facePoint));
            pointFaceMapping[face.vIndex[1]].push_back(cVA(facePoint));
            pointFaceMapping[face.vIndex[2]].push_back(cVA(facePoint));
            pointFaceMapping[face.vIndex[3]].push_back(cVA(facePoint));

            for (int i = 0; i < 4; ++i) {
                int v1 = face.vIndex[i];
                int v2 = face.vIndex[(i + 1) % 4];
                glm::vec3 edgePoint = (glm::vec3(model.gVertices[v1].x, model.gVertices[v1].y, model.gVertices[v1].z) + 
                                    glm::vec3(model.gVertices[v2].x, model.gVertices[v2].y, model.gVertices[v2].z)) * 0.5f;


                
                edgePoints.push_back(cVA(edgePoint));
    
                edgePointMapping[cVA(edgePoint)].push_back(v1);
                edgePointMapping[cVA(edgePoint)].push_back(v2);

                edgeToFacePointMapping[cVA(edgePoint)].push_back(cVA(facePoint));
            }
        
        }
    }
    else
    {
        //FOR TRIANGLES 
        for (auto face : model.gFaces_3) {
            glm::vec3 facePoint(0.0f);
            for (int i = 0; i < 3 ; ++i) {
                int vertexIndex = face.vIndex[i];
                facePoint += glm::vec3(model.gVertices[vertexIndex].x, 
                model.gVertices[vertexIndex].y, model.gVertices[vertexIndex].z);
            }
            facePoint /= 3.0f;
            subdividedModel.gVertices.push_back({ facePoint.x, facePoint.y, facePoint.z });
            indexesOfVertices[cVA(facePoint)] = subdividedModel.gVertices.size()-1;
            
            //use in summation in the P calculation
            pointFaceMapping[face.vIndex[0]].push_back(cVA(facePoint));
            pointFaceMapping[face.vIndex[1]].push_back(cVA(facePoint));
            pointFaceMapping[face.vIndex[2]].push_back(cVA(facePoint));

            for (int i = 0; i < 3; ++i) {
                int v1 = face.vIndex[i];
                int v2 = face.vIndex[(i + 1) % 3];
                glm::vec3 edgePoint = (glm::vec3(model.gVertices[v1].x, model.gVertices[v1].y, model.gVertices[v1].z) + 
                                    glm::vec3(model.gVertices[v2].x, model.gVertices[v2].y, model.gVertices[v2].z)) * 0.5f;


                
                edgePoints.push_back(cVA(edgePoint));
    
                edgePointMapping[cVA(edgePoint)].push_back(v1);
                edgePointMapping[cVA(edgePoint)].push_back(v2);

                edgeToFacePointMapping[cVA(edgePoint)].push_back(cVA(facePoint));
            }
        
        }
    }


    // std::cout<<"CORE DUMPED -> " << segmentFault++ <<endl;





    for (int i = 0; i < edgePoints.size(); i++)
    {
        glm::vec3 edgePoint = cAV(edgePoints[i]);
        glm::vec3 midEdgePoint = edgePoint;

        vector<int> vertices = edgePointMapping[cVA(edgePoint)];

        vector<vector<float>> facePoints = edgeToFacePointMapping[cVA(edgePoint)];
    
        edgePoint = edgePoint + (cAV(facePoints[0]) + cAV(facePoints[1])) * 0.5f;
        edgePoint = edgePoint * 0.5f;
        
        edgePoints[i] = cVA(edgePoint);

        

        
        Vertex newVert = { edgePoint.x, edgePoint.y, edgePoint.z };

        if (!containsVertex(subdividedModel.gVertices, newVert)) {
            subdividedModel.gVertices.push_back(newVert);
            indexesOfVertices[cVA(edgePoint)] = subdividedModel.gVertices.size()-1;

            pointEdgeMapping[vertices[0]].push_back(cVA(edgePoint));
            pointEdgeMapping[vertices[1]].push_back(cVA(edgePoint));
            pointMidEdgeMapping[vertices[0]].push_back(cVA(midEdgePoint));
            pointMidEdgeMapping[vertices[1]].push_back(cVA(midEdgePoint));
        }

        
        faceToEdgePointMapping[facePoints[0]].push_back(cVA(edgePoint));
        faceToEdgePointMapping[facePoints[1]].push_back(cVA(edgePoint));
    }
    
    
    
    cout<<"MODEL FACE_3 COUNT => "<<model.gFaces_3.size()<<endl;
    cout<<"MODEL FACE_4 COUNT => "<<model.gFaces_4.size()<<endl;
    cout<<"MODEL VERTEX COUNT => "<<model.gVertices.size()<<endl;
    for (int vertexIndex = 0; vertexIndex < model.gVertices.size(); vertexIndex++)
    {
        int n = pointFaceMapping[vertexIndex].size();
        glm::vec3 vertexPoint(0.0f);
        
        glm::vec3 F(0.0f);
        glm::vec3 R(0.0f);

        for(auto facePoint: pointFaceMapping[vertexIndex])
        {
            F += cAV(facePoint);
        }
        F = F / float(n);

        for(auto edgePoint: pointMidEdgeMapping[vertexIndex])
        {
            R += cAV(edgePoint);
        }
        R = R / float(pointMidEdgeMapping[vertexIndex].size());
        
        
        vertexPoint = F + 2.0f * R + float(n-3) * glm::vec3(model.gVertices[vertexIndex].x, model.gVertices[vertexIndex].y, model.gVertices[vertexIndex].z);
        vertexPoint = vertexPoint / float(n);

        subdividedModel.gVertices.push_back({ vertexPoint.x, vertexPoint.y, vertexPoint.z });
        indexesOfVertices[cVA(vertexPoint)] = subdividedModel.gVertices.size()-1;


        vector<vector<float>> edgePointsF = pointEdgeMapping[vertexIndex];
        vector<vector<float>> facePointsF = pointFaceMapping[vertexIndex];
        
        cout<<"COUNT ->>> "<< facePointsF.size()* model.gVertices.size()<<endl ;

        for(auto facePointF: facePointsF)
        {

            vector<vector<float>> edgesOfFace = faceToEdgePointMapping[facePointF];
            vector<vector<float>> edgesFinal;

            // printVectorFloat(edgesOfFace);
            // cout<<"-----------------------"<<endl;

            for(auto edge: edgesOfFace)
            {
                auto res = find(edgePointsF.begin(), edgePointsF.end(), edge);
                auto rem = find(edgesFinal.begin(), edgesFinal.end(), edge);
                if (res != edgePointsF.end() && rem == edgesFinal.end()) {
                    edgesFinal.push_back(edge);
                } 
            }

            
            
            int edgeId1 = indexesOfVertices[edgesFinal[0]];
            int edgeId2 = indexesOfVertices[edgesFinal[1]];
            int vertexId = indexesOfVertices[cVA(vertexPoint)];
            int faceId = indexesOfVertices[facePointF];



            
            int verticeIds[3] = {faceId, edgeId1, vertexId};
            int textures[3];
            int normals[3];

            Face3 newFace(verticeIds, textures, normals);

            subdividedModel.gFaces_3.push_back(newFace);

            int verticeIds2[3] = {faceId, vertexId, edgeId2};

            Face3 newFace2(verticeIds2, textures, normals);

            subdividedModel.gFaces_3.push_back(newFace2);


            int verticeIds_4[4] = {faceId, edgeId1, vertexId, edgeId2};
            int textures_4[4];
            int normals_4[4] = {faceId, edgeId1, vertexId, edgeId2};
            Face4 newFace_4(verticeIds_4, textures_4, normals_4);
            
            cout<<"-------------VERTICES-------------"<<endl;
            cout<<subdividedModel.gVertices[edgeId1].x << " : " << subdividedModel.gVertices[edgeId1].y << " : "<< subdividedModel.gVertices[edgeId1].z<<endl
            <<subdividedModel.gVertices[edgeId2].x << " : "<< subdividedModel.gVertices[edgeId2].y << " : "<<subdividedModel.gVertices[edgeId2].z<<endl
            <<subdividedModel.gVertices[vertexId].x << " : "<< subdividedModel.gVertices[vertexId].y << " : "<<subdividedModel.gVertices[vertexId].z<<endl
            <<subdividedModel.gVertices[faceId].x << " : " << subdividedModel.gVertices[faceId].y << " : " <<subdividedModel.gVertices[faceId].z<<endl
            <<endl;

            subdividedModel.gFaces_4.push_back(newFace_4);
        }
        
    }
    

    

    cout<<"SUB MODEL FACE_3 COUNT => "<<subdividedModel.gFaces_3.size()<<endl;
    cout<<"SUB MODEL FACE_4 COUNT => "<<subdividedModel.gFaces_4.size()<<endl;
    cout<<"SUB MODEL VERTEX COUNT => "<<subdividedModel.gVertices.size()<<endl;
    
    
    model.gNormals.clear();
    
    computeVertexNormals(subdividedModel);
    model.gFaces_4 = subdividedModel.gFaces_4;
    model.gFaces_3 = subdividedModel.gFaces_3;
    model.gVertices = subdividedModel.gVertices;
    model.gNormals = subdividedModel.gNormals;
    

    computeVertexNormals(model);
    initVBO_four(model);
    initVBO(model);

    
}


void  reset()
{
    level = 0;

    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].model = initialObjects[i].model;
        objects[i].modelingMatrix = initialObjects[i].modelingMatrix;
        objects[i].position = initialObjects[i].position;
    }

    currentMode = VisualizationMode::Solid;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
}


void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl");
	GLuint fs2 = createFS("frag2.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	// Link the programs

    glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void init() 
{
    Model cubeM;
    Model tetM;
    Model octaM;


	//ParseObj("armadillo.obj");
	ParseObj("tetrahedron.obj",tetM,false);
    ParseObj("cube.obj", cubeM,true);
    ParseObj("octahedron.obj", octaM,false);

    

    computeVertexNormals(cubeM);
    computeVertexNormals(tetM);
    computeVertexNormals(octaM);

    printNormals(cubeM.gNormals);
    initVBO_four(cubeM);
    initVBO_four(tetM);
    initVBO_four(octaM);

    initVBO(cubeM);
    initVBO(tetM);
    initVBO(octaM);




    
    cube.model = cubeM;
    cube.modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(5.0f, 0.0f, -distanceFromCam));
    cube.position = glm::vec3(2.0f, -0.4f, -10.0f);

    tetrahedron.model = tetM;
    tetrahedron.modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -distanceFromCam));
    tetrahedron.position = glm::vec3(-2.0f, -0.4f, -10.0f);

    octahedron.model = octaM;
    octahedron.modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -5.0f, -distanceFromCam));
    octahedron.position = glm::vec3(-2.0f, -0.4f, -10.0f);

    
    objects.push_back(tetrahedron);
    objects.push_back(cube);
    objects.push_back(octahedron); 

    initialObjects.push_back(tetrahedron);
    initialObjects.push_back(cube);
    initialObjects.push_back(octahedron);

    vector<Model> t;
    t.push_back(tetrahedron.model);     
    oldModels.push_back(t);

    vector<Model> c;
    c.push_back(cube.model);
    oldModels.push_back(c); 

    vector<Model> o;
    o.push_back(octahedron.model);
    oldModels.push_back(o); 


    glEnable(GL_DEPTH_TEST);
    initShaders();  
}


void RotateAroundPoint(SceneObject &object, float rotationAmount, glm::vec3 pointPos, glm::vec3 rotationAxis)
{
    
	// Compute the modeling matrix
    glm::mat4 modelingMatrix;    

    
    glm::quat rotationQuat = glm::angleAxis(rotationAmount, rotationAxis); // Rotate around the y-axis

    modelingMatrix = glm::translate(glm::mat4(1.0f), pointPos) *
                    glm::toMat4(rotationQuat) *
                    glm::translate(glm::mat4(1.0f), -pointPos) *
                    object.modelingMatrix;

    object.modelingMatrix = modelingMatrix;
}

void drawModel(Model &model , bool poly)
{

    if(model.gFaces_4.size() > 0 && !poly && (currentMode == VisualizationMode::Line || currentMode == VisualizationMode::Wire))
    {
        glBindBuffer(GL_ARRAY_BUFFER, model.gVAB_four);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        glDrawArrays(GL_LINES, 0,model.gFaces_4.size() *4*2);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, model.gVAB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.gIB);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(model.gVertSize));
        glDrawElements(GL_TRIANGLES, model.gFaces_3.size() * 3, GL_UNSIGNED_INT, 0);
    }
        

    
    
}

void display(SceneObject &object, bool poly)
{

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(object.modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
    drawModel(object.model, poly);

}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glOrtho(-10, 10, -10, 10, -10, 10);
    //gluPerspective(45, 1, 1, 100);

	// Use perspective projection

	float fovyRad = (float) (45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, 1.0f, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)

	viewingMatrix = glm::mat4(1);

    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
}

void changeVisualizationMode() {

    switch (currentMode) 
    {
        case VisualizationMode::Solid: // to line mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            currentMode = VisualizationMode::Line;
            break;
        case VisualizationMode::Line: // to wire mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            currentMode = VisualizationMode::Wire;
            break;
        case VisualizationMode::Wire: // to solid mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            currentMode = VisualizationMode::Solid;
            break;
    }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        //glShadeModel(GL_SMOOTH);
        activeProgramIndex = 0;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        //glShadeModel(GL_SMOOTH);
        activeProgramIndex = 1;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        
    }

    if(key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        //TODO DECREASE LEVEL
        if(level > 0)
        {
            level -= 1;

            for (int i = 0; i < objects.size(); i++)
            {

                objects[i].model = oldModels[i][level];

            }
        }

    }
    else if(key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        //TODO INCREASE LEVEL
        level += 1;

        for (int i = 0; i < objects.size(); i++)
        {
            if(level >= oldModels[i].size())
            {
                

                if(objects[i].model.gFaces_4.size()>0)
                {
                    subDivision(objects[i].model, true);
                }
                else
                    subDivision(objects[i].model, false);

                oldModels[i].push_back(objects[i].model);
            }
            else
            {
                objects[i].model = oldModels[i][level];
            }
        }
    }

    
    //CHANGE VISUALIZATION MODE
    if(key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        changeVisualizationMode();
    }

    if(key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        //TODO RESET
        reset();
    }
    if(key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        //TODO starts/pauses animations
        areAnimsActive = !areAnimsActive;
    }
 
}

void RenderScene(GLFWwindow* window)
{
    
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glDepthMask(GL_TRUE); 
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            
    

    if (currentMode == VisualizationMode::Line)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthMask(GL_TRUE); 
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 

        
        glClear(GL_DEPTH_BUFFER_BIT);

        display(objects[0],true);
        display(objects[1],true);
        display(objects[2],true);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }



    if (currentMode == VisualizationMode::Line)
    {
        
        glDepthMask(GL_FALSE); 
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 

        
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        
    }
    

    // Set the color uniform
    GLuint colorLoc = glGetUniformLocation(gProgram[activeProgramIndex], "objectColor");
    glUniform3f(colorLoc, 1.5f, 0.0f, 0.0f); 


    display(objects[0],false);
    glUniform3f(colorLoc, 0.0f, 2.0f, 0.0f); 
    display(objects[1],false);
    glUniform3f(colorLoc, 0.0f, 0.0f, 2.0f); 
    display(objects[2],false);

    if (currentMode == VisualizationMode::Line)
        glDisable(GL_POLYGON_OFFSET_LINE); 


    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        
        if(areAnimsActive)
        {
            RotateAroundPoint(objects[0], 0.01f, glm::vec3(0.0f, 0.0f, -distanceFromCam), glm::vec3(-1, 1, 0));
            RotateAroundPoint(objects[1], 0.01f, glm::vec3(0.0f, 0.0f, -distanceFromCam), glm::vec3(0, 0, -1));
            RotateAroundPoint(objects[2], 0.01f, glm::vec3(0.0f, 0.0f, -distanceFromCam), glm::vec3(1, 0, 0));
        }
        RenderScene(window);      
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int width = 640, height = 480;
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    
    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
