#include "structure.h"
#include <fstream>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))



void initVBO_four(Model &model);



bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

bool ParseObj(const string& fileName, Model& model, bool isCube)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        model.gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        model.gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        model.gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    if(isCube)
                    {
                        str >> tmp; // consume "f"
                        char c;
                        int vIndex[4],  nIndex[4], tIndex[4];
                        str >> vIndex[0]; str >> c >> c; // consume "//"
                        str >> nIndex[0]; 
                        str >> vIndex[1]; str >> c >> c; // consume "//"
                        str >> nIndex[1]; 
                        str >> vIndex[2]; str >> c >> c; // consume "//"
                        str >> nIndex[2]; 
                        str >> vIndex[3]; str >> c >> c; // consume "//"
                        str >> nIndex[3];

                        assert(vIndex[0] == nIndex[0] &&
                            vIndex[1] == nIndex[1] &&
                            vIndex[2] == nIndex[2]); // a limitation for now

                        // make indices start from 0
                        for (int c = 0; c < 4; ++c)
                        {
                            vIndex[c] -= 1;
                            nIndex[c] -= 1;
                            tIndex[c] -= 1;
                        }
                        int f1_index[3] = {vIndex[0],vIndex[1],vIndex[2]};
                        int f2_index[3] = {vIndex[2],vIndex[3],vIndex[0]};
                        int f_4_indexes[4] = {vIndex[0],vIndex[1],vIndex[2], vIndex[3]};

                        model.gFaces_3.push_back(Face3(f1_index, tIndex, nIndex));
                        model.gFaces_3.push_back(Face3(f2_index, tIndex, nIndex));

                        model.gFaces_4.push_back(Face4(f_4_indexes,tIndex, nIndex));
                    }
                    else
                    {
                        str >> tmp; // consume "f"
                        char c;
                        int vIndex[3],  nIndex[3], tIndex[3];
                        str >> vIndex[0]; str >> c >> c; // consume "//"
                        str >> nIndex[0]; 
                        str >> vIndex[1]; str >> c >> c; // consume "//"
                        str >> nIndex[1]; 
                        str >> vIndex[2]; str >> c >> c; // consume "//"
                        str >> nIndex[2]; 

                        assert(vIndex[0] == nIndex[0] &&
                            vIndex[1] == nIndex[1] &&
                            vIndex[2] == nIndex[2]); // a limitation for now

                        // make indices start from 0
                        for (int c = 0; c < 3; ++c)
                        {
                            vIndex[c] -= 1;
                            nIndex[c] -= 1;
                            tIndex[c] -= 1;
                        }

                        model.gFaces_3.push_back(Face3(vIndex, tIndex, nIndex));
                    }

                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	
	// for (int i = 0; i < model.gVertices.size(); ++i)
	// {
	// 	glm::vec3 n;

	// 	for (int j = 0; j < gFaces.size(); ++j)
	// 	{
	// 		for (int k = 0; k < 3; ++k)
	// 		{
	// 			if (gFaces[j].vIndex[k] == i)
	// 			{
	// 				// face j contains vertex i
	// 				glm::vec3 a(gVertices[gFaces[j].vIndex[0]].x, 
	// 						  gVertices[gFaces[j].vIndex[0]].y,
	// 						  gVertices[gFaces[j].vIndex[0]].z);

	// 				glm::vec3 b(gVertices[gFaces[j].vIndex[1]].x, 
	// 						  gVertices[gFaces[j].vIndex[1]].y,
	// 						  gVertices[gFaces[j].vIndex[1]].z);

	// 				glm::vec3 c(gVertices[gFaces[j].vIndex[2]].x, 
	// 						  gVertices[gFaces[j].vIndex[2]].y,
	// 						  gVertices[gFaces[j].vIndex[2]].z);

	// 				glm::vec3 ab = b - a;
	// 				glm::vec3 ac = c - a;


	// 				glm::vec3 normalFromThisFace = (ab.cross(ac)).getNormalized();
	// 				n += normalFromThisFace;
	// 			}

	// 		}
	// 	}

	// 	n.normalize();

	// 	gNormals.push_back(Normal(n.x, n.y, n.z));
	// }
	
    model.gNormals.clear();
    computeVertexNormals(model);
	assert(model.gVertices.size() == model.gNormals.size());
    
    return true;
}



GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);
    
	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}


void initVBO(Model &model)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &model.gVAB);
    
	glGenBuffers(1, &model.gIB);

	assert(model.gVAB > 0 && model.gIB > 0);

	glBindBuffer(GL_ARRAY_BUFFER, model.gVAB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.gIB);

	model.gVertSize = model.gVertices.size() * 3 * sizeof(GLfloat);
	model.gNormalSize = model.gNormals.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = model.gFaces_3.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat [model.gVertices.size() * 3];
	GLfloat* normalData = new GLfloat [model.gNormals.size() * 3];
	GLuint* indexData = new GLuint [model.gFaces_3.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < model.gVertices.size(); ++i)
	{
		vertexData[3*i] = model.gVertices[i].x;
		vertexData[3*i+1] = model.gVertices[i].y;
		vertexData[3*i+2] = model.gVertices[i].z;

        minX = std::min(minX, model.gVertices[i].x);
        maxX = std::max(maxX, model.gVertices[i].x);
        minY = std::min(minY, model.gVertices[i].y);
        maxY = std::max(maxY, model.gVertices[i].y);
        minZ = std::min(minZ, model.gVertices[i].z);
        maxZ = std::max(maxZ, model.gVertices[i].z);
	}

    // std::cout << "minX = " << minX << std::endl;
    // std::cout << "maxX = " << maxX << std::endl;
    // std::cout << "minY = " << minY << std::endl;
    // std::cout << "maxY = " << maxY << std::endl;
    // std::cout << "minZ = " << minZ << std::endl;
    // std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < model.gNormals.size(); ++i)
	{
		normalData[3*i] = model.gNormals[i].x;
		normalData[3*i+1] = model.gNormals[i].y;
		normalData[3*i+2] = model.gNormals[i].z;
	}

	for (int i = 0; i < model.gFaces_3.size(); ++i)
	{
		indexData[3*i] = model.gFaces_3[i].vIndex[0];
		indexData[3*i+1] = model.gFaces_3[i].vIndex[1];
		indexData[3*i+2] = model.gFaces_3[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, model.gVertSize + model.gNormalSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, model.gVertSize, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, model.gVertSize, model.gNormalSize, normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(model.gVertSize));

    
}

void initVBO_four(Model &model)
{
    GLuint vao_four;
    glGenVertexArrays(1, &vao_four);
    assert(vao_four > 0);
    glBindVertexArray(vao_four);
    cout << "vao = " << vao_four << endl;

	glEnableVertexAttribArray(0);
	
	GLenum error = glGetError();
    if (error != GL_NONE) 
    {
        std::cerr << "OpenGL error: " << error << std::endl;
    }

	glGenBuffers(1, &model.gVAB_four);
    
	glBindBuffer(GL_ARRAY_BUFFER, model.gVAB_four);
	



	
	GLfloat* vertexData = new GLfloat [model.gFaces_4.size() * 4 * 3 * 2];
	
	

	for (int i = 0; i < model.gFaces_4.size(); ++i)
	{
        for (int vid = 0; vid < 4; vid++)
        {
            Vertex vert = model.gVertices[model.gFaces_4[i].vIndex[vid%4]];
            vertexData[8*3*i + vid * 6] = vert.x;
            vertexData[8*3*i + vid * 6 +1] = vert.y;
            vertexData[8*3*i + vid * 6 +2] = vert.z;

            vert = model.gVertices[model.gFaces_4[i].vIndex[(vid+1) %4]];
            vertexData[8*3*i + vid * 6 +3] = vert.x;
            vertexData[8*3*i + vid * 6 +4] = vert.y;
            vertexData[8*3*i + vid * 6 +5] = vert.z;
        }
    
	}






	glBufferData(GL_ARRAY_BUFFER, model.gFaces_4.size() * 4 * 3 * sizeof(GLfloat) * 2 , vertexData, GL_STATIC_DRAW);



	// done copying; can free now
	delete[] vertexData;

	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

