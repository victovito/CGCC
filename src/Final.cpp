/* Projeto Final - Victor Silva da Rosa */

/*
Controles:
    Câmera:
    WASD - Move a câmera nos eixos X e Z
    EQ - Move a câmera no eixo Y
    Scroll - Controla o zoom

    Objetos:
    1234567890 - Seleciona um objeto para manipular
    Setas - Move o objeto nos eixos X e Z
	IJ - Move o objeto no eixo Y
	[] - Modifica a escala
	XYZ - Rotaciona o objeto nos eixos XYZ
	T - Ativa/desativa a trajetória do objeto

    Luzes:
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

#pragma region Constantes

const string windowName = "Projeto Final - Victor Silva da Rosa";
const string controlPointsSource = "../src/control-points.txt";
const string vertexShaderSource = "../src/shaders/final/vertex.shader.glsl";
const string fragmentShaderSource = "../src/shaders/final/fragment.shader.glsl";

const GLuint WIDTH = 800, HEIGHT = 800;
const int MAX_OBJECTS = 10;
const int MAX_LIGHTS = 16;

const string meshes[] {
	"../meshes/Building.obj",
	"../meshes/Suzanne.obj",
	"../meshes/Suzanne.obj",
	"../meshes/Suzanne.obj",
	"../meshes/Fridge.obj",
	"../meshes/Car.obj",
	"../meshes/Cone.obj",
	"../meshes/Cone.obj",
	"../meshes/Cone.obj"
};

#pragma endregion

#pragma region Assinatura de funções

// Setups
int setupShader();
void setupCamera();
void setupObjects();
void setupLights();

// Updates
void updateCamera();
void updateObjects();
void updateLights();
void renderScene(GLuint shaderID);

// Inputs
void scroll_callback(GLFWwindow *window, double x, double y);
void cursor_pos_callback(GLFWwindow *window, double x, double y);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void resetInputs(GLFWwindow *window);

// Carregamento de recursos
void loadOBJ(string filePath, GLuint &VAO, int &nVertices, GLuint &texId, float &ka, float &kd, float &ks, float &ns);
GLuint loadTexture(string filePath, int &width, int &height);
string loadFile(string filePath);
vector<vec3> loadControlPoints(int objIndex);
bool getMaterialFromMtl(string filePath, string &textureName, float &ka, float &kd, float &ks, float &ns);

// Extras
vec3 catmullUniform(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t);

#pragma endregion

#pragma region Structs

struct Inputs
{
    // Controle de camera
	bool W;
	bool A;
	bool S;
	bool D;
    bool E;
    bool Q;
    vec2 mouseOffset;
    float scrollOffset;

	// Controle de objetos
    bool upArrow;
    bool leftArrow;
    bool downArrow;
    bool rightArrow;
	bool I;
	bool J;
	bool X;
	bool Y;
	bool Z;
	bool T;
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

	vector<vec3> controlPoints;
	bool followPath;
	float pathPosition;
};

struct Light {
    vec3 position;
    vec3 color;
	bool enabled;
};

struct Camera {
    vec3 position;
    vec3 forward;
    vec3 up;
    float fov;
};

#pragma endregion

#pragma region Variáveis globais

Inputs inputs;

Object objects[MAX_OBJECTS];
int objectsCount = 0;

Light lights[MAX_LIGHTS];
int lightsCount = 0;

Camera camera;

int selectedObject = 99;
float deltaTime = 0;
double lastFrame = 0;

#pragma endregion

int main()
{
	glfwInit();

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, windowName.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
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

    setupCamera();
	setupObjects();
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

	    deltaTime = (float)(glfwGetTime() - lastFrame);

        updateCamera();
		updateObjects();
		updateLights();
		
		lastFrame = glfwGetTime();
		resetInputs(window);

		renderScene(shaderID);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
}

#pragma region Implementações de funções

// Setups

int setupShader()
{
	GLint success;
	GLchar infoLog[512];

	string vertexShaderCode = loadFile(vertexShaderSource);
	const char *vShaderSrc = vertexShaderCode.c_str();
	string fragmentShaderCode = loadFile(fragmentShaderSource);
	const char *fShaderSrc = fragmentShaderCode.c_str();

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

void setupCamera()
{
    camera = *(new Camera());
    camera.position = vec3(0, 0, 5);
    camera.forward = vec3(0, 0, -1);
    camera.up = vec3(0, 1, 0);
    camera.fov = 90;
}

void setupObjects()
{
	for (int i = 0; i < size(meshes); i++)
	{
		Object obj;

		obj.controlPoints = loadControlPoints(i);
		obj.followPath = obj.controlPoints.size() > 1;
		loadOBJ(meshes[i], obj.VAO, obj.nVertex, obj.texId, obj.ka, obj.kd, obj.ks, obj.ns);

		obj.position = obj.controlPoints.empty() ? vec3() : obj.controlPoints[0];
		obj.rotation = vec3(0, 0, 0);
		obj.scale = 1;

		objects[i] = obj;
		objectsCount++;
	}
}

void setupLights()
{
	Light keyLight;
	keyLight.position = vec3(-10, 0, 5);
	keyLight.color = vec3(255, 255, 220) / 255.0f * 10.0f;
	keyLight.enabled = true;
	lights[0] = keyLight;
	
	Light fillLight;
	fillLight.position = vec3(15, 0, 10);
	fillLight.color = vec3(150, 150, 129) / 255.0f * 10.0f;
	fillLight.enabled = true;
	lights[1] = fillLight;
	
	Light backLight;
	backLight.position = vec3(-20, 0, -20);
	backLight.color = vec3(179, 93, 23) / 255.0f * 20.0f;
	backLight.enabled = true;
	lights[2] = backLight;

	lightsCount = 3;
}

// Updates

void updateCamera()
{    
    float sensitivity = 0.005f;
    float zoomSensitivity = 0.1f;
    float speed = 10;

    // Mouse look
    float pitch = asinf(camera.forward.y);
    float yaw = atan2f(-camera.forward.x, -camera.forward.z);

    pitch = radians(clamp(degrees(pitch - inputs.mouseOffset.y * sensitivity), -89.9f, 89.9f));
    yaw -= inputs.mouseOffset.x * sensitivity;

    camera.forward = normalize(vec3(
        -sin(yaw) * cos(pitch),
        sin(pitch),
        -cos(yaw) * cos(pitch)
    ));
    
    vec3 right = normalize(cross(camera.forward, vec3(0, 1, 0)));

    camera.up = normalize(cross(right, camera.forward));

    // Camera movement
    vec3 moveInput = vec3((float)inputs.D - inputs.A, 0, (float)inputs.W - inputs.S);
    float upDownInput = inputs.E - inputs.Q;

    camera.position += vec3(0, upDownInput * speed * deltaTime, 0);

    if (length(moveInput) > 0) {
        vec3 basisRight = normalize(cross(camera.forward, camera.up));
        mat3 basis = mat3(
            vec3(basisRight.x, camera.up.x, camera.forward.x),
            vec3(basisRight.y, camera.up.y, camera.forward.y),
            vec3(basisRight.z, camera.up.z, camera.forward.z)
        );
        vec3 moveDirection = inverse(basis) * moveInput;
    
        camera.position += normalize(moveDirection) * speed * deltaTime;
    }

    // Zoom
    camera.fov = clamp(camera.fov * (1 - inputs.scrollOffset * zoomSensitivity), 20.0f, 120.0f);
}

void updateObjects()
{
	for (int i = 0; i < objectsCount; i++) {
		Object current = objects[i];

		if (current.followPath && current.controlPoints.size() > 3)
		{
			int pointCount = current.controlPoints.size();

			current.pathPosition = fmod(current.pathPosition + deltaTime, (float)pointCount);

			int segment = (int)floor(current.pathPosition);
			float t = fract(current.pathPosition);

			vec3 p0 = current.controlPoints[(segment - 1 + pointCount) % pointCount];
			vec3 p1 = current.controlPoints[(segment + 0) % pointCount];
			vec3 p2 = current.controlPoints[(segment + 1) % pointCount];
			vec3 p3 = current.controlPoints[(segment + 2) % pointCount];

			current.position = catmullUniform(p0, p1, p2, p3, t);

			objects[i] = current;
		}
	}

    if (selectedObject > objectsCount - 1) return;

	Object selected = objects[selectedObject];

	if (inputs.T) {
		selected.followPath = !selected.followPath;
	}

	if (!selected.followPath) {
		vec3 posOffset = vec3(inputs.rightArrow - inputs.leftArrow, inputs.I - inputs.J, inputs.downArrow - inputs.upArrow) * deltaTime;
		selected.position += posOffset;
	}

	vec3 rotOffset = vec3(inputs.X, inputs.Y, inputs.Z) * deltaTime;
	float scaleOffset = (inputs.rightBracket - inputs.leftBracket) * deltaTime;

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
    GLint viewLoc = glGetUniformLocation(shaderId, "view");
	GLint modelLoc = glGetUniformLocation(shaderId, "model");
	GLint lightsCountLoc = glGetUniformLocation(shaderId, "lightsCount");
	GLint kaLoc = glGetUniformLocation(shaderId, "ka");
	GLint kdLoc = glGetUniformLocation(shaderId, "kd");
	GLint ksLoc = glGetUniformLocation(shaderId, "ks");
	GLint shininessLoc = glGetUniformLocation(shaderId, "shininess");

	mat4 projection = perspective(radians(camera.fov), (float)WIDTH / HEIGHT, 0.01f, 1000.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    mat4 view = lookAt(camera.position, camera.position + camera.forward, camera.up);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));

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

// Inputs

void scroll_callback(GLFWwindow *window, double x, double y)
{
    inputs.scrollOffset += y;
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    inputs.mouseOffset += vec2(x, y);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

    // Camera
	if (key == GLFW_KEY_W)
		inputs.W = action;
	if (key == GLFW_KEY_A)
		inputs.A = action;
	if (key == GLFW_KEY_S)
		inputs.S = action;
	if (key == GLFW_KEY_D)
		inputs.D = action;
	if (key == GLFW_KEY_E)
		inputs.E = action;
	if (key == GLFW_KEY_Q)
		inputs.Q = action;
    
    // Objects
    if (key == GLFW_KEY_UP)
        inputs.upArrow = action;
    if (key == GLFW_KEY_LEFT)
        inputs.leftArrow = action;
    if (key == GLFW_KEY_DOWN)
        inputs.downArrow = action;
    if (key == GLFW_KEY_RIGHT)
        inputs.rightArrow = action;
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
	if (key == GLFW_KEY_T)
		inputs.T = action == GLFW_PRESS;
	if (key == GLFW_KEY_LEFT_BRACKET)
		inputs.leftBracket = action;
	if (key == GLFW_KEY_RIGHT_BRACKET)
		inputs.rightBracket = action;
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

    // Lights
	if (key == GLFW_KEY_K)
		inputs.K = action == GLFW_PRESS;
	if (key == GLFW_KEY_F)
		inputs.F = action == GLFW_PRESS;
	if (key == GLFW_KEY_B)
		inputs.B = action == GLFW_PRESS;
	
}

void resetInputs(GLFWwindow *window) {
	inputs.K = false;
	inputs.F = false;
	inputs.B = false;
	inputs.T = false;

    inputs.mouseOffset = vec3(0);
    inputs.scrollOffset = 0;

    glfwSetCursorPos(window, 0, 0);
}

// Carregamento de recursos

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

vector<vec3> loadControlPoints(int objIndex) {
	ifstream file(controlPointsSource.c_str());
	string line;

	bool found = false;
	vector<vec3> points;

	while (getline(file, line))
	{
		istringstream ssline(line);
		string word;
		ssline >> word;

		if (word == "obj") {
			if (found) break;

			int index;
			ssline >> index;
			if (index == objIndex) {
				found = true;
			}
		}

		if (found) {
			if (word == "p") {
				vec3 point;
				ssline >> point.x >> point.y >> point.z;
				points.push_back(point);
			}
		}
	}

	return points;
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

// Extras

vec3 catmullUniform(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
    );
}

#pragma endregion
