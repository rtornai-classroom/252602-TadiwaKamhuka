// vertex array
enum eVertexArrayObject {
	VAOVerticesData,
	VAOCount
};
//buffer for vertices and colors
enum eBufferObject {
	VBOVerticesData,
	VBOColorData,
	BOCount
};
enum eProgram {
	QuadScreenProgram,
	ProgramCount
};
// types of animations
enum eAnimationType {
	Vertical,
	Horizontal,
	Diagonal,
	None
};

#include <array>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#define PI 3.14159265358979323846   // pi

using namespace	std;
using namespace glm;

const GLchar* windowTitle = "Task 1";

typedef struct {
	GLenum       type;
	const GLchar* fileName;
	GLuint       shader;
} ShaderInfo;

// initial values for enum counts
GLuint			VAO[VAOCount] = { 0 };
GLuint			BO[BOCount] = { 0 };
GLuint			program[ProgramCount] = { 0 };

// window meta info
GLint windowWidth = 700;
GLint windowHeight = 700;
GLboolean keyboard[512] = { GL_FALSE };
GLFWwindow* window = nullptr;

// swapColor uniform in fragShader location
GLuint swapLocation;

GLuint			locationMatProjection, locationMatModelView;
mat4			matModel, matView, matProjection, matModelView;

// circle information
const GLint segments = 100;
array<vec2, (segments + 4)> vertices;
array<vec3, (segments + 4)> colors;
GLfloat radius = 0.25;

// information for animation
GLuint			XoffsetLocation;
GLuint			YoffsetLocation;
GLfloat			x = 0.00f;
GLfloat			y = 0.00f;
GLfloat			lineY = 0.00f;
GLfloat			lineDisp = 0.02f;
GLfloat			increment = 0.01f;
GLfloat			vx = increment * cos(radians(35.0));
GLfloat			vy = increment * sin(radians(35.0));

eAnimationType	animationType = None;



void cleanUpScene(int returnCode) {
    glDeleteVertexArrays(VAOCount, VAO);
    glDeleteBuffers(BOCount, BO);
    for (int enumItem = 0; enumItem < ProgramCount; enumItem++)
		glDeleteProgram(program[enumItem]);
    glfwTerminate();
    exit(returnCode);
}

GLboolean checkOpenGLError() {
	GLboolean	foundError = GL_FALSE;
	GLenum		glErr;
	while ((glErr = glGetError()) != GL_NO_ERROR) {
		cerr << "glError: " << gluErrorString(glErr) << endl;
		foundError = GL_TRUE;
	}
	return foundError;
}

GLvoid checkShaderLog(GLuint shader) {
	GLint	compiled;
	GLint	length = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled) return;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length > 0) {
		GLchar* log = (GLchar*)calloc((size_t)length + 1, sizeof(GLchar));
		glGetShaderInfoLog(shader, length, nullptr, log);
		cerr << "Shader compiling failed, info log: " << log << endl;
		free(log);
	}
	cleanUpScene(EXIT_FAILURE);
}

GLvoid checkProgramLog(GLint prog, ShaderInfo* shaders) {
	GLint	linked;
	GLint	length = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked) return;
	for (ShaderInfo* entry = shaders; entry->type != GL_NONE; ++entry) {
		glDeleteShader(entry->shader);
		entry->shader = 0;
	}
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
	if (length > 0) {
		GLchar* log = (GLchar*)calloc((size_t)length + 1, sizeof(GLchar));
		glGetProgramInfoLog(prog, length, nullptr, log);
		cerr << "Shader linking failed, info log: " << log << endl;
		free(log);
	}
	cleanUpScene(EXIT_FAILURE);
}

string ReadShader(const GLchar* filename) {
	ifstream		t(filename);
	stringstream	buffer;
	if (t.fail()) cleanUpScene(EXIT_FAILURE);
	buffer << t.rdbuf();
	return buffer.str();
}

GLuint LoadShaders(ShaderInfo* shaders) {
	if (shaders == nullptr) return 0;
	GLuint		program = glCreateProgram();
	ShaderInfo* entry = shaders;
	while (entry->type != GL_NONE) {
		GLuint			shader = glCreateShader(entry->type);
		string			sourceString = ReadShader(entry->fileName);
		const GLchar* source = sourceString.c_str();

		entry->shader = shader;
		if (source == nullptr) {
			for (entry = shaders; entry->type != GL_NONE; ++entry) {
				glDeleteShader(entry->shader);
				entry->shader = 0;
			}
			cleanUpScene(EXIT_FAILURE);
		}
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		checkShaderLog(shader);
		glAttachShader(program, shader);
		++entry;
	}
	glLinkProgram(program);
	checkProgramLog(program, shaders);
	for (entry = shaders; entry->type != GL_NONE; ++entry) {
		glDeleteShader(entry->shader);
		entry->shader = 0;
	}
	return program;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);

	float aspectRatio = (float)windowWidth / (float)windowHeight;
	glViewport(0, 0, windowWidth, windowHeight);

	matModel = mat4(1.0);
	matView = lookAt(
		vec3(0.0f, 0.0f, 9.0f),	
		vec3(0.0f, 0.0f, 0.0f),	
		vec3(0.0f, 1.0f, 0.0f));	
	matModelView = matView * matModel;
	glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;

	// keep final position and move circle from that position
	// diagonal movement
	if (key == GLFW_KEY_S && action == GLFW_PRESS) { 
		animationType = Diagonal;
		//x = x;
		//y = y;
	}
	// vertical movement
	if (key == GLFW_KEY_V && action == GLFW_PRESS) { 
		animationType = Vertical;
		//x = x;
		//y = y;
	}
	// horizontal movement
	if (key == GLFW_KEY_H && action == GLFW_PRESS) { 
		animationType = Horizontal;
		//x = x;
		//y = y;
	}
	// stop movement at
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		animationType = None;
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) lineY = 0.0;
}

void init(GLint major, GLint minor) {
	if (!glfwInit()) cleanUpScene(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

	if ((window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr)) == nullptr) {
		cerr << "Failed to create GLFW window." << endl;
		cleanUpScene(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	if (glewInit() != GLEW_OK) {
		cerr << "Failed to init the GLEW system." << endl;
		cleanUpScene(EXIT_FAILURE);
	}
	glfwSwapInterval(1);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwSetWindowPos(window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);
	glGenBuffers(BOCount, BO);
	glGenVertexArrays(VAOCount, VAO);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(8.0);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	cout.precision(2);
	cout << fixed;
}

void initShaderProgram() {
	ShaderInfo shader_info[] = {
		{ GL_FRAGMENT_SHADER,	"src/fragmentShader.glsl" },
		{ GL_VERTEX_SHADER,		"src/vertexShader.glsl" },
		{ GL_NONE,				nullptr }
	};
	
	program[QuadScreenProgram] = LoadShaders(shader_info);
	glBindVertexArray(VAO[VAOVerticesData]);
	//////////////////// Position Buffer /////////////////////
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//////////////////// Color Buffer /////////////////////
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOColorData]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	XoffsetLocation = glGetUniformLocation(program[QuadScreenProgram], "offsetX");
	YoffsetLocation = glGetUniformLocation(program[QuadScreenProgram], "offsetY");
	swapLocation = glGetUniformLocation(program[QuadScreenProgram], "swapColors");

	glUseProgram(program[QuadScreenProgram]);
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);

	matModel = mat4(1.0);
	switch (animationType) {
		case None:
			glProgramUniform1f(program[QuadScreenProgram], XoffsetLocation, x);
			glProgramUniform1f(program[QuadScreenProgram], YoffsetLocation, y);
			break;
		case Horizontal:
			x += vx;
			if ((x + radius > 1.0) || (x - radius < -1.0)) vx = -vx;
			glProgramUniform1f(program[QuadScreenProgram], XoffsetLocation, x);
			glProgramUniform1f(program[QuadScreenProgram], YoffsetLocation, y);
			break;
		case Vertical:
			y += vy;
			if ((y + radius > 1.0) || (y - radius < -1.0)) vy = -vy;
			glProgramUniform1f(program[QuadScreenProgram], XoffsetLocation, x);
			glProgramUniform1f(program[QuadScreenProgram], YoffsetLocation, y);
			break;
		case Diagonal:
			x += vx;
			y += vy;
			if ((x + radius > 1.0) || (x - radius < -1.0)) vx = -vx;
			if ((y + radius > 1.0) || (y - radius < -1.0)) vy = -vy;
			glProgramUniform1f(program[QuadScreenProgram], XoffsetLocation, x);
			glProgramUniform1f(program[QuadScreenProgram], YoffsetLocation, y);
			break;
	}
	bool intersect = abs(y - lineY) <= radius &&
		(x + radius >= -0.25f && x - radius <= 0.25f);

	glUniform1i(swapLocation, intersect);
	
	glDrawArrays(GL_TRIANGLE_FAN, 0, (segments + 2));

	matModelView = matView * matModel;
	glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));

	if (keyboard[GLFW_KEY_UP] && lineY < 0.99f)lineY += lineDisp;

	if (keyboard[GLFW_KEY_DOWN] && lineY > -0.99) lineY -= lineDisp;

	glProgramUniform1f(program[QuadScreenProgram], XoffsetLocation, 0.0f);
	glProgramUniform1f(program[QuadScreenProgram], YoffsetLocation, lineY);

	glDrawArrays(GL_LINES, (segments + 2), 2);
}

int main(void){
	init(3, 3);

    vertices[0] = vec2(0.0, 0.0);
	colors[0] = vec3(1.0, 0.0, 0.0);

    for (int i = 0; i <= segments; i++) {
        GLfloat angle = 2.0 * PI * i / segments;
        GLfloat x = radius * cos(angle);
        GLfloat y = radius * sin(angle);
        vertices[i + 1] = vec2(x, y);
		colors[i + 1] = vec3(0.0, 1.0, 0.0);
    }
	
	GLint lineStart = segments + 2;
	vertices[lineStart] = vec2(-0.25f, 0.0f);
	colors[lineStart] = vec3(0.0, 0.0, 1.0);
	vertices[lineStart + 1] = vec2(0.25f, 0.0f);
	colors[lineStart + 1] = vec3(0.0, 0.0, 1.0);

	initShaderProgram();

	cout << "Keyboard control" << endl;
	cout << "ESC\t\texit" << endl;
	cout << "H\t\tFor horizontal circle movement" << endl;
	cout << "Q\t\tTo stop movement and return circle to center" << endl;
	cout << "R\t\tReset line position to middle of screen" << endl;
	cout << "S\t\tStart circle's 35 degree movement (from center)" << endl;
	cout << "V\t\tFor vertical circle movement" << endl;
	cout << "Up Arrow\tFor moving line up" << endl;
	cout << "Down Arrow\tFor moving line down" << endl;

    while (!glfwWindowShouldClose(window))
    {
		display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	cleanUpScene(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}