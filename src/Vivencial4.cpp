/* Atividade vivencial do Módulo 2 - Victor Silva da Rosa */

/*
Controles:
	1234567890 - Seleciona um objeto para manipular
	WASD - Move nos eixos X e Z
	IJ - Move no eixo Y
	[] - Modifica a escala
	XYZ - Rotaciona nos eixos XYZ
	KFB - Ativa/desativa as luzes (key light, fill light e backlight, respectivamente)
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <format>
#include <vector>
#include <assert.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
using namespace glm;

const GLuint WIDTH = 800, HEIGHT = 800;
const int MAX_OBJECTS = 10;
const int MAX_LIGHTS = 16;

const string meshes[] {
	"../meshes/Suzanne.obj",
	"../meshes/Suzanne.obj",
	"../meshes/Suzanne.obj",
};

void loadOBJ(string filePath, GLuint &VAO, int &nVertices, GLuint &texId, float &ka, float &kd, float &ks, float &ns);
GLuint loadTexture(string filePath, int &width, int &height);
string loadFile(string filePath);
bool getMaterialFromMtl(string filePath, string &textureName, float &ka, float &kd, float &ks, float &ns);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void resetInputs();
int setupShader();
void setupMeshes();
void setupLights();
void updateObjects();
void updateLights();
void renderScene(GLuint shaderID);

struct Inputs
{
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

	// Controle de luzes
	bool K;
	bool F;
	bool B;
};

struct Object
{
	GLuint VAO;
	int nVertex;
	vec3 position;
	vec3 rotation;
	float scale;

	GLuint texId;
	float ka, kd, ks, ns;
};

struct Light {
    vec3 position;
    vec3 color;
	bool enabled;
};

Inputs inputs;

Object objects[MAX_OBJECTS];
int objectsCount = 0;

Light lights[MAX_LIGHTS];
int lightsCount = 0;

int selectedObject = 0;
float lastFrame = 0;

int main()
{
	glfwInit();

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial 4 - Victor Silva da Rosa", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	setupMeshes();
	setupLights();

	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "mainTexture"), 0);
	glEnable(GL_DEPTH_TEST);

	lastFrame = (float)glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0, 0, 0, 0); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateObjects();
		updateLights();
		
		lastFrame = (float)glfwGetTime();
		resetInputs();

		renderScene(shaderID);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

	if (key == GLFW_KEY_W)
		inputs.W = action;
	if (key == GLFW_KEY_A)
		inputs.A = action;
	if (key == GLFW_KEY_S)
		inputs.S = action;
	if (key == GLFW_KEY_D)
		inputs.D = action;
	if (key == GLFW_KEY_I)
		inputs.I = action;
	if (key == GLFW_KEY_J)
		inputs.J = action;
	if (key == GLFW_KEY_X)
		inputs.X = action;
	if (key == GLFW_KEY_Y)
		inputs.Y = action;
	if (key == GLFW_KEY_Z)
		inputs.Z = action;
	if (key == GLFW_KEY_LEFT_BRACKET)
		inputs.leftBracket = action;
	if (key == GLFW_KEY_RIGHT_BRACKET)
		inputs.rightBracket = action;

	if (key == GLFW_KEY_K)
		inputs.K = action == GLFW_PRESS;
	if (key == GLFW_KEY_F)
		inputs.F = action == GLFW_PRESS;
	if (key == GLFW_KEY_B)
		inputs.B = action == GLFW_PRESS;
	
	if (key == GLFW_KEY_1)
		selectedObject = 0;
	if (key == GLFW_KEY_2)
		selectedObject = 1;
	if (key == GLFW_KEY_3)
		selectedObject = 2;
	if (key == GLFW_KEY_4)
		selectedObject = 3;
	if (key == GLFW_KEY_5)
		selectedObject = 4;
	if (key == GLFW_KEY_6)
		selectedObject = 5;
	if (key == GLFW_KEY_7)
		selectedObject = 6;
	if (key == GLFW_KEY_8)
		selectedObject = 7;
	if (key == GLFW_KEY_9)
		selectedObject = 8;
	if (key == GLFW_KEY_0)
		selectedObject = 9;
	
	selectedObject = selectedObject % MAX_OBJECTS;
}

void resetInputs() {
	inputs.K = false;
	inputs.F = false;
	inputs.B = false;
}

void setupMeshes()
{
	for (int i = 0; i < size(meshes); i++)
	{
		Object obj;
		obj.position = vec3(0, 0, -5);
		obj.rotation = vec3(0, 0, 0);
		obj.scale = 1;
		loadOBJ(meshes[i], obj.VAO, obj.nVertex, obj.texId, obj.ka, obj.kd, obj.ks, obj.ns);

		objects[i] = obj;
		objectsCount++;
	}
}

void setupLights()
{
	Light keyLight;
	keyLight.position = vec3(-2, 0, -1);
	keyLight.color = vec3(255, 255, 220) / 255.0f;
	keyLight.enabled = true;
	lights[0] = keyLight;
	
	Light fillLight;
	fillLight.position = vec3(3, 0, -2);
	fillLight.color = vec3(150, 150, 129) / 255.0f;
	fillLight.enabled = true;
	lights[1] = fillLight;
	
	Light backLight;
	backLight.position = vec3(-4, 0, -8);
	backLight.color = vec3(179, 93, 23) / 255.0f;
	backLight.enabled = true;
	lights[2] = backLight;

	lightsCount = 3;
}

void updateObjects()
{
	float delta = (float)glfwGetTime() - lastFrame;
	Object selected = objects[selectedObject];

	vec3 posOffset = vec3(inputs.D - inputs.A, inputs.I - inputs.J, inputs.S - inputs.W) * delta;
	vec3 rotOffset = vec3(inputs.X, inputs.Y, inputs.Z) * delta;
	float scaleOffset = (inputs.rightBracket - inputs.leftBracket) * delta;

	selected.position += posOffset;
	selected.rotation += rotOffset;
	selected.scale += scaleOffset;

	objects[selectedObject] = selected;
}

void updateLights()
{
	if (inputs.K) lights[0].enabled = !lights[0].enabled;
	if (inputs.F) lights[1].enabled = !lights[1].enabled;
	if (inputs.B) lights[2].enabled = !lights[2].enabled;
}

void renderScene(GLuint shaderId)
{
	GLint projectionLoc = glGetUniformLocation(shaderId, "projection");
	GLint modelLoc = glGetUniformLocation(shaderId, "model");
	GLint lightsCountLoc = glGetUniformLocation(shaderId, "lightsCount");
	GLint kaLoc = glGetUniformLocation(shaderId, "ka");
	GLint kdLoc = glGetUniformLocation(shaderId, "kd");
	GLint ksLoc = glGetUniformLocation(shaderId, "ks");
	GLint shininessLoc = glGetUniformLocation(shaderId, "shininess");

	mat4 projection = perspective(radians(75.0f), (float)WIDTH / HEIGHT, 0.01f, 1000.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

	glUniform1i(lightsCountLoc, lightsCount);

	for (int i = 0; i < lightsCount; i++)
	{
		Light light = lights[i];

		string lightLoc = string("lights[") + to_string(i) + string("].");
		
		GLint positionLoc = glGetUniformLocation(shaderId, (lightLoc + string("position")).c_str());
		GLint colorLoc = glGetUniformLocation(shaderId, (lightLoc + string("color")).c_str());
		GLint enabledLoc = glGetUniformLocation(shaderId, (lightLoc + string("enabled")).c_str());

		glUniform3fv(positionLoc, 1, value_ptr(light.position));
		glUniform3fv(colorLoc, 1, value_ptr(light.color));
		glUniform1i(enabledLoc, light.enabled);
	}

	for (int i = 0; i < objectsCount; i++)
	{
		Object obj = objects[i];

		mat4 model = mat4(1);

		model = translate(model, obj.position);

		model = rotate(model, obj.rotation.z, vec3(0, 0, 1));
		model = rotate(model, obj.rotation.y, vec3(0, 1, 0));
		model = rotate(model, obj.rotation.x, vec3(1, 0, 0));

		model = scale(model, vec3(obj.scale));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
		
		glUniform1f(kaLoc, obj.ka);
		glUniform1f(kdLoc, obj.kd);
		glUniform1f(ksLoc, obj.ks);
		glUniform1f(shininessLoc, obj.ns);

		glBindVertexArray(obj.VAO);
		glBindTexture(GL_TEXTURE_2D, obj.texId);
		glDrawArrays(GL_TRIANGLES, 0, obj.nVertex);

		if (selectedObject == i)
		{
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

	string vertexShaderSource = loadFile("../src/shaders/vivencial4/vertex.shader.glsl");
	const char *vShaderSrc = vertexShaderSource.c_str();
	string fragmentShaderSource = loadFile("../src/shaders/vivencial4/fragment.shader.glsl");
	const char *fShaderSrc = fragmentShaderSource.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderSrc, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
			 << infoLog << endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderSrc, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
			 << infoLog << endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
			 << infoLog << endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void loadOBJ(string filePath, GLuint &VAO, int &nVertices, GLuint &texId, float &ka, float &kd, float &ks, float &ns)
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
		return;
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
			vt.t = 1 - vt.t;
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

				if (getline(ss, index, '/'))
					vi = !index.empty() ? stoi(index) - 1 : 0;
				if (getline(ss, index, '/'))
					ti = !index.empty() ? stoi(index) - 1 : 0;
				if (getline(ss, index))
					ni = !index.empty() ? stoi(index) - 1 : 0;

				vBuffer.push_back(vertices[vi].x);
				vBuffer.push_back(vertices[vi].y);
				vBuffer.push_back(vertices[vi].z);

				vBuffer.push_back(texCoords[ti].s);
				vBuffer.push_back(texCoords[ti].t);

				vBuffer.push_back(normals[ni].x);
				vBuffer.push_back(normals[ni].y);
				vBuffer.push_back(normals[ni].z);
			}
		}
		else if (word == "mtllib")
		{
			filesystem::path dir = filesystem::path(filePath).parent_path();
			
			filesystem::path name;
			ssline >> name;
			
			filesystem::path mtl = dir/name;
			
			string textureName;

            if (getMaterialFromMtl(mtl.string(), textureName, ka, kd, ks, ns))
			{
				int w, h;
				texId = loadTexture((dir/textureName).string(), w, h);
			}
		}
	}

	file.close();

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (GLvoid *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	nVertices = vBuffer.size() / 8;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

string loadFile(string filePath)
{
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

bool getMaterialFromMtl(string filePath, string &textureName, float &ka, float &kd, float &ks, float &ns)
{
    ifstream file(filePath.c_str());

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            istringstream ssline(line);
            string word;
            ssline >> word;

            if (word == "map_Kd")
            {
                ssline >> textureName;
				return true;
            }
			if (word == "Ka")
			{
				ssline >> ka;
			}
			if (word == "Kd")
			{
				ssline >> kd;
			}
			if (word == "Ks")
			{
				ssline >> ks;
			}
			if (word == "Ns")
			{
				ssline >> ns;
			}
        }
    }

	return false;
}
