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



using namespace std;

enum VisualizationMode{
    Solid,
    Line,
    Wire
};

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face3
{
	Face3(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct Face4
{
	Face4(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
        vIndex[3] = v[3];

		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
        tIndex[3] = t[3];

		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
        nIndex[3] = n[3];
	}
    GLuint vIndex[4], tIndex[4], nIndex[4];
};



struct Model{
    vector<Vertex> gVertices;
    vector<Normal> gNormals;
    vector<Face3> gFaces_3;
    vector<Face4> gFaces_4;
    vector<Texture> gTextures;

    GLuint gVAB, gIB;
    GLuint vao;
    int gVertSize, gNormalSize;


    GLuint gVAB_four;
};


struct SceneObject{

    SceneObject(Model _model)
    {
        model = _model;
    }

    SceneObject()
    {
       
    }
    
    Model model;
    glm::mat4 modelingMatrix;
    glm::vec3 position;
    glm::vec3 rotation;
};


void printNormals(vector<Normal> normals)
{
    for(auto normal: normals)
    {
        cout<<normal.x << " : " << normal.y << " : " << normal.z<<endl;
    }
}

void computeVertexNormals(Model& model) {

    
    model.gNormals.clear();
    for (int i = 0; i < model.gVertices.size(); ++i)
	{
		glm::vec3 n;

		for (int j = 0; j < model.gFaces_3.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (model.gFaces_3[j].vIndex[k] == i)
				{
					// face j contains vertex i
					glm::vec3 a(model.gVertices[model.gFaces_3[j].vIndex[0]].x, 
							  model.gVertices[model.gFaces_3[j].vIndex[0]].y,
							  model.gVertices[model.gFaces_3[j].vIndex[0]].z);

					glm::vec3 b(model.gVertices[model.gFaces_3[j].vIndex[1]].x, 
							  model.gVertices[model.gFaces_3[j].vIndex[1]].y,
							  model.gVertices[model.gFaces_3[j].vIndex[1]].z);

					glm::vec3 c(model.gVertices[model.gFaces_3[j].vIndex[2]].x, 
							  model.gVertices[model.gFaces_3[j].vIndex[2]].y,
							  model.gVertices[model.gFaces_3[j].vIndex[2]].z);

					glm::vec3 ab = b - a;
					glm::vec3 ac = c - b;

                    
					glm::vec3 normalFromThisFace = (glm::cross(ab,ac));
                    normalFromThisFace = glm::normalize(normalFromThisFace);
					n += normalFromThisFace;
				}

			}
		}

		n = glm::normalize(n);

        
		model.gNormals.push_back(Normal(n.x, n.y, n.z));

        
	}
    cout<<"NORMALS COUNT "<< model.gNormals.size()<<endl;
};