/* Atividade vivencial do Módulo 2 - Victor Silva da Rosa */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

const GLuint WIDTH = 800, HEIGHT = 800;
const int MAX_OBJECTS = 10;

const string meshes[] {
    "../meshes/Cube.obj",
    "../meshes/Suzanne.obj",
};

int loadSimpleOBJ(string filePath, int &nVertices);
string loadFile(string filePath);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
void setupMeshes();
void updateMeshes();
void renderMeshes(GLint modelLoc);

struct Inputs {
    // Mover
    bool W;
    bool A;
    bool S;
    bool D;
    bool I;
    bool J;

    // Rotacionar
    bool X;
    bool Y;
    bool Z;

    // Escalonar
    bool leftBracket;
    bool rightBracket;
};

struct Object {
    GLuint VAO;
    int nVertex;
    vec3 position;
    vec3 rotation;
    float scale;
};

Inputs inputs;
Object objects[MAX_OBJECTS];
int objectsCount = 0;
int selectedObject = 0;
float lastFrame = 0;

int main() {
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial 2 - Victor Silva da Rosa", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

    setupMeshes();

    glUseProgram(shaderID);
	glEnable(GL_DEPTH_TEST);

    lastFrame = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();
        
		// Limpa o buffer de cor
		glClearColor(0, 0, 0, 0); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		updateMeshes();
		lastFrame = (float)glfwGetTime();

        renderMeshes(modelLoc);
        
		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

    if (key == GLFW_KEY_W) inputs.W = action;
    if (key == GLFW_KEY_A) inputs.A = action;
    if (key == GLFW_KEY_S) inputs.S = action;
    if (key == GLFW_KEY_D) inputs.D = action;
    if (key == GLFW_KEY_I) inputs.I = action;
    if (key == GLFW_KEY_J) inputs.J = action;
    if (key == GLFW_KEY_X) inputs.X = action;
    if (key == GLFW_KEY_Y) inputs.Y = action;
    if (key == GLFW_KEY_Z) inputs.Z = action;
    if (key == GLFW_KEY_LEFT_BRACKET) inputs.leftBracket = action;
    if (key == GLFW_KEY_RIGHT_BRACKET) inputs.rightBracket = action;

    if (key == GLFW_KEY_1) selectedObject = 0;
    if (key == GLFW_KEY_2) selectedObject = 1;
    if (key == GLFW_KEY_3) selectedObject = 2;
    if (key == GLFW_KEY_4) selectedObject = 3;
    if (key == GLFW_KEY_5) selectedObject = 4;
    if (key == GLFW_KEY_6) selectedObject = 5;
    if (key == GLFW_KEY_7) selectedObject = 6;
    if (key == GLFW_KEY_8) selectedObject = 7;
    if (key == GLFW_KEY_9) selectedObject = 8;
    if (key == GLFW_KEY_0) selectedObject = 9;
    
    selectedObject = selectedObject % MAX_OBJECTS;
}

void setupMeshes() {
    for (int i = 0; i < size(meshes); i++) {
        Object obj;
        obj.VAO = loadSimpleOBJ(meshes[i], obj.nVertex);
        obj.position = vec3(0, 0, 0);
        obj.rotation = vec3(0, 0, 0);
        obj.scale = .5f;

        objects[i] = obj;
        objectsCount++;
    }
}

void updateMeshes() {
    float delta = (float)glfwGetTime() - lastFrame;
    Object selected = objects[selectedObject];

    vec3 posOffset = vec3(inputs.D - inputs.A, inputs.I - inputs.J, inputs.W - inputs.S) * delta;
    vec3 rotOffset = vec3(inputs.X, inputs.Y, inputs.Z) * delta;
    float scaleOffset = (inputs.rightBracket - inputs.leftBracket) * delta;

    selected.position += posOffset;
    selected.rotation += rotOffset;
    selected.scale += scaleOffset;
    
    objects[selectedObject] = selected;
}

void renderMeshes(GLint modelLoc) {
    for (int i = 0; i < objectsCount; i++) {
        Object obj = objects[i];

        mat4 model = mat4(1);

        model = translate(model, obj.position);
        
		model = rotate(model, obj.rotation.z, vec3(0, 0, 1));
		model = rotate(model, obj.rotation.y, vec3(0, 1, 0));
		model = rotate(model, obj.rotation.x, vec3(1, 0, 0));

		model = scale(model, vec3(obj.scale));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

		glBindVertexArray(obj.VAO);
		glDrawArrays(GL_TRIANGLES, 0, obj.nVertex);

        if (selectedObject == i) {
            glPointSize(10);
            glDrawArrays(GL_POINTS, 0, obj.nVertex);
        }

		glBindVertexArray(0);
    }
}

int setupShader()
{
	GLint success;
	GLchar infoLog[512];

    string vertexShaderSource = loadFile("../src/shaders/vivencial2/vertex.shader.glsl");
    const char* vShaderSrc = vertexShaderSource.c_str();
    string fragmentShaderSource = loadFile("../src/shaders/vivencial2/fragment.shader.glsl");
    const char* fShaderSrc = fragmentShaderSource.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderSrc, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}
    
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderSrc, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}
    
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int loadSimpleOBJ(string filePath, int &nVertices)
 {
    vector<vec3> vertices;
    vector<vec2> texCoords;
    vector<vec3> normals;
    vector<GLfloat> vBuffer;
    vec3 color = vec3(1.0, 0.0, 0.0);

    ifstream file(filePath.c_str());

    if (!file.is_open()) 
	{
        cerr << "Erro ao tentar ler o arquivo " << filePath << endl;
        return -1;
    }

    string line;
    while (getline(file, line)) 
	{
        istringstream ssline(line);
        string word;
        ssline >> word;

        if (word == "v") 
		{
            vec3 vertice;
            ssline >> vertice.x >> vertice.y >> vertice.z;
            vertices.push_back(vertice);
        } 
        else if (word == "vt") 
		{
            vec2 vt;
            ssline >> vt.s >> vt.t;
            texCoords.push_back(vt);
        } 
        else if (word == "vn") 
		{
            vec3 normal;
            ssline >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } 
        else if (word == "f")
		 {
            while (ssline >> word) 
			{
                int vi = 0, ti = 0, ni = 0;
                istringstream ss(word);
                string index;

                if (getline(ss, index, '/')) vi = !index.empty() ? stoi(index) - 1 : 0;
                if (getline(ss, index, '/')) ti = !index.empty() ? stoi(index) - 1 : 0;
                if (getline(ss, index)) ni = !index.empty() ? stoi(index) - 1 : 0;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);

                vBuffer.push_back(normals[ni].x);
                vBuffer.push_back(normals[ni].y);
                vBuffer.push_back(normals[ni].z);
                
                vBuffer.push_back(texCoords[ti].x);
                vBuffer.push_back(texCoords[ti].y);
            }
        }
    }

    file.close();

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	nVertices = vBuffer.size() / 8;

    return VAO;
}

string loadFile(string filePath) {
    ifstream file(filePath.c_str());

    if (!file.is_open()) 
	{
        cerr << "Erro ao tentar ler o arquivo " << filePath << endl;
        return "";
    }

    stringstream buffer;
    buffer.clear();
    buffer << file.rdbuf();

    file.close();
    string content = buffer.str();

    return content;
}
