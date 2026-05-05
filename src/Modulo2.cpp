/* Tarefa do Módulo 2 - Victor Silva da Rosa */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 800;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = R"(
#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 model;

out vec4 finalColor;

void main() {
	gl_Position = model * vec4(position, 1.0);
	finalColor = vec4(color, 1.0);
}
)";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = R"(
#version 450

in vec4 finalColor;
out vec4 color;

void main() {
	color = finalColor;
}
)";


float lastFrame = 0;
float scale = 0.5f;
glm::vec3 position(0, 0, 0);

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 2 - Victor Silva da Rosa", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();

	glUseProgram(shaderID);
	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();
		lastFrame = (float)glfwGetTime();

		// Limpa o buffer de cor
		glClearColor(0, 0, 0, 0); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model;

		// Cubo principal
		model = glm::mat4(1);
		model = glm::translate(model, position);
		model = glm::rotate(model, lastFrame, glm::vec3(1));
		model = glm::scale(model, glm::vec3(scale));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		
		// Outros cubos
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.5f, 0, 0));
		model = glm::rotate(model, lastFrame + (float)(M_PI * 2 / 3), glm::vec3(-1, 1, -1));
		model = glm::scale(model, glm::vec3(0.5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-0.5f, 0, 0));
		model = glm::rotate(model, lastFrame + (float)(M_PI * 4 / 3), glm::vec3(-1, 1, 1));
		model = glm::scale(model, glm::vec3(0.5));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}

	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);

	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();

	return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}

	float deltaTime = (float)glfwGetTime() - lastFrame;

	if (key == GLFW_KEY_W) position.z += deltaTime;
	if (key == GLFW_KEY_S) position.z -= deltaTime;

	if (key == GLFW_KEY_A) position.x -= deltaTime;
	if (key == GLFW_KEY_D) position.x += deltaTime;

	if (key == GLFW_KEY_I) position.y += deltaTime;
	if (key == GLFW_KEY_J) position.y -= deltaTime;

	if (key == GLFW_KEY_LEFT_BRACKET) scale -= deltaTime;
	if (key == GLFW_KEY_RIGHT_BRACKET) scale += deltaTime;

	position = glm::vec3(
		max(-1.f, min(1.f, position.x)),
		max(-1.f, min(1.f, position.y)),
		max(-1.f, min(1.f, position.z))
	);
	scale = max(0.2f, min(1.f, scale));

}

int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupGeometry()
{
	
	GLfloat vertices[] = {

		//x    y    z       r    g    b
		-0.5, -0.5, -0.5,	1.0, 1.0, 1.0,
		0.5, -0.5, -0.5,	1.0, 1.0, 1.0,
		-0.5, -0.5, 0.5,	1.0, 1.0, 1.0,
		-0.5, -0.5, 0.5,	1.0, 1.0, 1.0,
		0.5, -0.5, -0.5,	1.0, 1.0, 1.0,
		0.5, -0.5, 0.5,		1.0, 1.0, 1.0,

		-0.5, 0.5, -0.5,	1.0, 1.0, 0.0,
		-0.5, 0.5, 0.5,		1.0, 1.0, 0.0,
		0.5, 0.5, -0.5,		1.0, 1.0, 0.0,
		-0.5, 0.5, 0.5,		1.0, 1.0, 0.0,
		0.5, 0.5, 0.5,		1.0, 1.0, 0.0,
		0.5, 0.5, -0.5,		1.0, 1.0, 0.0,


		-0.5, -0.5, 0.5,	1.0, 0.0, 0.0,
		0.5, -0.5, 0.5,		1.0, 0.0, 0.0,
		0.5, 0.5, 0.5,		1.0, 0.0, 0.0,
		-0.5, -0.5, 0.5,	1.0, 0.0, 0.0,
		0.5, 0.5, 0.5,		1.0, 0.0, 0.0,
		-0.5, 0.5, 0.5,		1.0, 0.0, 0.0,

		-0.5, -0.5, -0.5,	1.0, 0.5, 0.0,
		0.5, 0.5, -0.5,		1.0, 0.5, 0.0,
		0.5, -0.5, -0.5,	1.0, 0.5, 0.0,
		-0.5, -0.5, -0.5,	1.0, 0.5, 0.0,
		-0.5, 0.5, -0.5,	1.0, 0.5, 0.0,
		0.5, 0.5, -0.5,		1.0, 0.5, 0.0,


		0.5, -0.5, 0.5,		0.0, 1.0, 0.0,
		0.5, -0.5, -0.5,	0.0, 1.0, 0.0,
		0.5, 0.5, -0.5,		0.0, 1.0, 0.0,
		0.5, -0.5, 0.5,		0.0, 1.0, 0.0,
		0.5, 0.5, -0.5,		0.0, 1.0, 0.0,
		0.5, 0.5, 0.5,		0.0, 1.0, 0.0,

		-0.5, -0.5, 0.5,	0.0, 0.0, 1.0,
		-0.5, 0.5, -0.5,	0.0, 0.0, 1.0,
		-0.5, -0.5, -0.5,	0.0, 0.0, 1.0,
		-0.5, -0.5, 0.5,	0.0, 0.0, 1.0,
		-0.5, 0.5, 0.5,		0.0, 0.0, 1.0,
		-0.5, 0.5, -0.5,	0.0, 0.0, 1.0,

	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

