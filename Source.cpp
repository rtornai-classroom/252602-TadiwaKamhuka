enum eVertexArrayObject {
	VAOCurveData,
	VAOCount
};
enum eVertexBufferObject {
	VBOBezierData,
	BOCount
};
enum eProgram {
	CurveTesselationProgram,
	QuadScreenProgram,
	ProgramCount
};
enum eTexture {
	NoTexture,
	TextureCount
};

#include "common.cpp"

#define	BEZIER_GMT			1
#define MAX_CONTROL_POINTS 16


GLchar	windowTitle[] = "Task 2 - Q9X9KP";
GLfloat	bezier_control_points[MAX_CONTROL_POINTS][3] = {
	{ -0.5f, -0.5f, 0.0f }, { -0.5f,  0.5f, 0.0f },
	{  0.5f,  0.5f, 0.0f }, {  0.5f, -0.5f, 0.0f }
};

GLuint			locationTessMatProjection, locationTessMatModelView, locationCurveType, locationControlPointsNumber;
GLuint			curveType = BEZIER_GMT, controlPointsNumber = 4;

GLuint			locationColor;
GLint			selectedPoint = -1;
GLboolean		dragging = false;

glm::vec2 screenToWorld(GLFWwindow* window, double x, double y) {
	float nx = (float)x / windowWidth * 2.0f - 1.0f;
	float ny = 1.0f - (float)y / windowHeight * 2.0f;

	glm::vec4 clip(nx, ny, 0, 1);

	glm::mat4 inv = glm::inverse(matProjection * matModelView);
	glm::vec4 world = inv * clip;

	return glm::vec2(world.x, world.y);
}

void updateVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOBezierData]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * MAX_CONTROL_POINTS, bezier_control_points, GL_DYNAMIC_DRAW);
}

void initTesselationShader() {
	ShaderInfo shader_info[] = {
		{ GL_FRAGMENT_SHADER,			"./CurveFragShader.glsl" },
		{ GL_TESS_CONTROL_SHADER,		"./CurveTessContShader.glsl" },
		{ GL_TESS_EVALUATION_SHADER,	"./CurveTessEvalShader.glsl" },
		{ GL_VERTEX_SHADER,				"./CurveVertShader.glsl" },
		{ GL_NONE,						nullptr }
	};

	ShaderInfo point_shader_info[] = {
		{ GL_VERTEX_SHADER,   "./QuadScreenVertShader.glsl" },
		{ GL_FRAGMENT_SHADER, "./QuadScreenFragShader.glsl" },
		{ GL_NONE, nullptr }
	};
	program[CurveTesselationProgram] = LoadShaders(shader_info);
	program[QuadScreenProgram] = LoadShaders(point_shader_info);
	glBindVertexArray(VAO[VAOCurveData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOBezierData]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bezier_control_points), bezier_control_points, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	locationCurveType = glGetUniformLocation(program[CurveTesselationProgram], "curveType");
	locationControlPointsNumber = glGetUniformLocation(program[CurveTesselationProgram], "controlPointsNumber");
	locationTessMatProjection = glGetUniformLocation(program[CurveTesselationProgram], "matProjection");
	locationTessMatModelView = glGetUniformLocation(program[CurveTesselationProgram], "matModelView");
	locationColor = glGetUniformLocation(program[QuadScreenProgram], "uColor");
	glUseProgram(program[CurveTesselationProgram]);
	glUniform1i(locationCurveType, curveType);
	glUniform1i(locationControlPointsNumber, controlPointsNumber);
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VAO[VAOCurveData]);
	glUseProgram(program[CurveTesselationProgram]);
	glUniform1i(locationControlPointsNumber, controlPointsNumber);
	glPatchParameteri(GL_PATCH_VERTICES, controlPointsNumber);
	glDrawArrays(GL_PATCHES, 0, controlPointsNumber);


	glUseProgram(program[QuadScreenProgram]);

	glUniform4f(locationColor, 0.0f, 0.0f, 1.0f, 1.0f);
	glDrawArrays(GL_LINE_STRIP, 0, controlPointsNumber);

	glUniform4f(locationColor, 1.0f, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_POINTS, 0, controlPointsNumber);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);

	float aspectRatio = (float)windowWidth / (float)windowHeight;
	glViewport(0, 0, windowWidth, windowHeight);
	if (windowWidth < windowHeight)
		matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
	else
		matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);

	matModel = mat4(1.0);
	matView = lookAt(
		vec3(0.0f, 0.0f, 9.0f),		
		vec3(0.0f, 0.0f, 0.0f),		
		vec3(0.0f, 1.0f, 0.0f));	
	matModelView = matView * matModel;
	glUseProgram(program[CurveTesselationProgram]);
	glUniformMatrix4fv(locationTessMatModelView, 1, GL_FALSE, glm::value_ptr(matModelView));
	glUniformMatrix4fv(locationTessMatProjection, 1, GL_FALSE, glm::value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {

	if (!dragging || selectedPoint == -1)
		return;

	glm::vec2 world = screenToWorld(window, xPos, yPos);

	bezier_control_points[selectedPoint][0] = world.x;
	bezier_control_points[selectedPoint][1] = world.y;

	updateVBO();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	glm::vec2 world = screenToWorld(window, x, y);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		dragging = false;
		selectedPoint = -1;
		return;
	}

	if (action != GLFW_PRESS) return;


	if (button == GLFW_MOUSE_BUTTON_LEFT) {

		selectedPoint = -1;

		for (int i = 0; i < controlPointsNumber; i++) {
			float dx = world.x - bezier_control_points[i][0];
			float dy = world.y - bezier_control_points[i][1];

			if (sqrt(dx * dx + dy * dy) < 0.1f) {
				selectedPoint = i;
				dragging = true;
				return;
			}
		}

		if (controlPointsNumber < MAX_CONTROL_POINTS) {
			bezier_control_points[controlPointsNumber][0] = world.x;
			bezier_control_points[controlPointsNumber][1] = world.y;
			bezier_control_points[controlPointsNumber][2] = 0.0f;

			controlPointsNumber++;

			updateVBO();
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		dragging = false;
		selectedPoint = -1;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {

		int closest = -1;
		float bestDist = 999999.0f;

		for (int i = 0; i < controlPointsNumber; i++) {
			float dx = world.x - bezier_control_points[i][0];
			float dy = world.y - bezier_control_points[i][1];
			float d = dx * dx + dy * dy;

			if (d < bestDist) {
				bestDist = d;
				closest = i;
			}
		}

		if (closest != -1 && controlPointsNumber > 4) {

			for (int i = closest; i < controlPointsNumber - 1; i++) {
				bezier_control_points[i][0] = bezier_control_points[i + 1][0];
				bezier_control_points[i][1] = bezier_control_points[i + 1][1];
				bezier_control_points[i][2] = bezier_control_points[i + 1][2];
			}

			controlPointsNumber--;
			updateVBO();
		}
	}


}

int main(void) {
	init(4, 0, GLFW_OPENGL_COMPAT_PROFILE); 
	initTesselationShader();
	setlocale(LC_ALL, "");
	cout << "Mouse control" << endl;
	cout << "Left click\tincrease control point number" << endl;
	cout << "Right click\tdecrease control point number" << endl << endl;
	framebufferSizeCallback(window, windowWidth, windowHeight);
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUpScene(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}