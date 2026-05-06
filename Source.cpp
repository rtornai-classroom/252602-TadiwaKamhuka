enum eVertexArrayObject {
	VAOVerticesData,
	VAOSun,
	VAOCount
};
enum eBufferObject {
	VBOVerticesData,
	EBOIndicesData,
	VBOSun,
	EBOSun,
	BOCount
};
enum eProgram {
	QuadScreenProgram,
	ProgramCount
};
enum eTexture {
	NoTexture,		// fixes 0 sized array problem
	TextureCount
};

#include "common.cpp"

GLchar					windowTitle[] = "Task 3 - Q9X9KP";
vec3					cameraTarget	= vec3(0.0f, 0.0f, 0.0f), 
						cameraUpVector	= vec3(0.0f, 1.0f, 0.0f);
float					cameraRadius = 10.0f;   // distance from origin
float					cameraAngle = 1.5f;    // rotation around Y axis
float					cameraHeight = 2.0f;    // vertical position
vec3					cameraPosition;

static vector<vec3>		cube_vertices = {
	vec3(-0.5f, -0.5f, -0.5f),
	vec3(0.5f, -0.5f, -0.5f),
	vec3(0.5f,  0.5f, -0.5f),
	vec3(-0.5f,  0.5f, -0.5f),
	vec3(-0.5f, -0.5f,  0.5f),
	vec3(0.5f, -0.5f,  0.5f),
	vec3(0.5f,  0.5f,  0.5f),
	vec3(-0.5f,  0.5f,  0.5f)
};
static vector<GLuint>	cube_indices = {
	// back
	0,1,2, 2,3,0,
	// front
	4,5,6, 6,7,4,
	// left
	0,4,7, 7,3,0,
	// right
	1,5,6, 6,2,1,
	// bottom
	0,1,5, 5,4,0,
	// top
	3,2,6, 6,7,3
};

GLuint					sunTexture;
bool					lightEnabled = false;
bool					magentaEnabled = false;
bool					showLightSphere = true;
float					lightRadius = 6.0f;
float					lightAngle = 0.0f;
vec3					lightPosition;
vec3					lightColor = vec3(1.0f, 1.0f, 0.0f);
GLuint					locationLightPos;
GLuint					locationLightColor;
GLuint					locationLightEnabled;
GLuint					locationMagentaEnabled;
GLuint					locationUseTexture;
double					lastX = 0, lastY = 0;
bool					firstMouse = true;
vector<vec3>			sphere_vertices;
vector<GLuint>			sphere_indices;

void initTextures() {
	sunTexture = SOIL_load_OGL_texture(
		"sun.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
	);

	if (!sunTexture) {
		cout << "SOIL error: " << SOIL_last_result() << endl;
	}

	glBindTexture(GL_TEXTURE_2D, sunTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void generateSphere(int stacks, int slices)
{
	sphere_vertices.clear();
	sphere_indices.clear();

	for (int i = 0; i <= stacks; i++) {
		float phi = glm::pi<float>() * i / stacks;

		for (int j = 0; j <= slices; j++) {
			float theta = 2.0f * glm::pi<float>() * j / slices;

			float x = sin(phi) * cos(theta);
			float y = cos(phi);
			float z = sin(phi) * sin(theta);

			sphere_vertices.push_back(vec3(x, y, z));
		}
	}

	for (int i = 0; i < stacks; i++) {
		for (int j = 0; j < slices; j++) {

			int first = i * (slices + 1) + j;
			int second = first + slices + 1;

			sphere_indices.push_back(first);
			sphere_indices.push_back(second);
			sphere_indices.push_back(first + 1);

			sphere_indices.push_back(second);
			sphere_indices.push_back(second + 1);
			sphere_indices.push_back(first + 1);
		}
	}
}

void initShaderProgram() {
	ShaderInfo shader_info[ProgramCount][3] = { {
		{ GL_VERTEX_SHADER,		"./vertexShader.glsl" },
		{ GL_FRAGMENT_SHADER,	"./fragmentShader.glsl" },
		{ GL_NONE, nullptr } } };

	for (int programItem = 0; programItem < ProgramCount; programItem++) {
		program[programItem] = LoadShaders(shader_info[programItem]);
		/** Shader változó location lekérdezése. */
		/** Getting shader variable location. */
		locationMatModel = glGetUniformLocation(program[programItem], "matModel");
		locationMatView = glGetUniformLocation(program[programItem], "matView");
		locationMatProjection = glGetUniformLocation(program[programItem], "matProjection");
	}
	/** Csatoljuk a vertex array objektumunkat a paraméterhez. */
	/** glBindVertexArray binds the vertex array object to the parameter. */
	glBindVertexArray(VAO[VAOVerticesData]);
	/** A GL_ARRAY_BUFFER nevesített csatolóponthoz kapcsoljuk a vertex buffert (ide kerülnek a csúcspont adatok). */
	/** We attach the vertex buffer to the GL_ARRAY_BUFFER node (vertices are stored here). */
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	/** Másoljuk az adatokat a bufferbe! Megadjuk az aktuálisan csatolt buffert, azt hogy hány byte adatot másolunk,
		a másolandó adatot, majd a feldolgozás módját is meghatározzuk: most az adat nem változik a feltöltés után. */
	/** Copy the data to the buffer! First parameter is the currently attached buffer, second is the size of the buffer to be copied,
		third is the array of data, fourth is working mode: now the data can not be modified after this step. */
	glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(vec3), cube_vertices.data(), GL_STATIC_DRAW);
	/** Ezen adatok szolgálják a location = 0 vertex attribútumot (itt: pozíció).
		Elsõként megadjuk ezt az azonosítószámot (vertexShader.glsl).
		Utána az attribútum méretét (vec3, láttuk a shaderben).
		Harmadik az adat típusa.
		Negyedik az adat normalizálása, ez maradhat FALSE jelen példában.
		Az attribútum értékek hogyan következnek egymás után? Milyen lépésköz után találom a következõ vertex adatait?
		Végül megadom azt, hogy honnan kezdõdnek az értékek a pufferben. Most rögtön, a legelejétõl veszem õket. */
	/** These values are for location = 0 vertex attribute (position).
		First is the location (vertexShader.glsl).
		Second is attribute size (vec3, as in the shader).
		Third is the data type.
		Fourth defines whether data shall be normalized or not, this is FALSE for the examples of the course.
		Fifth is the distance in bytes to the next vertex element of the array.
		Last is the offset of the first vertex data of the buffer. Now it is the start of the array. */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	/** Engedélyezzük az imént definiált location = 0 attribútumot (vertexShader.glsl ). */
	/** Enable the previously defined location = 0 attributum (vertexShader.glsl ). */
	glEnableVertexAttribArray(0);
	/** Beállítjuk a háromszögeket összekapcsoló indexeket. */
	/** Set indices for the triangles. */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BO[EBOIndicesData]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_indices.size() * sizeof(GLuint), cube_indices.data(), GL_STATIC_DRAW);

	generateSphere(20, 20);

	glGenVertexArrays(1, &VAO[VAOSun]);
	glGenBuffers(1, &BO[VBOSun]);
	glGenBuffers(1, &BO[EBOSun]);

	glBindVertexArray(VAO[VAOSun]);

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOSun]);
	glBufferData(GL_ARRAY_BUFFER,
		sphere_vertices.size() * sizeof(vec3),
		sphere_vertices.data(),
		GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOSun);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sphere_indices.size() * sizeof(GLuint),
		sphere_indices.data(),
		GL_STATIC_DRAW);

	// attribute 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glUseProgram(program[QuadScreenProgram]);
	/** Mátrixok kezdőértékének beállítása. */
	/** Set the matrices to the initial values. */
	projectionType = Perspective;
	matModel = mat4(1.0);
	matView = lookAt(
		cameraPosition,		// the position of your camera, in world space
		cameraTarget,		// where you want to look at, in world space
		cameraUpVector);	// upVector, probably vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
	glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
	glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
	locationLightPos = glGetUniformLocation(program[0], "lightPos");
	locationUseTexture = glGetUniformLocation(program[0], "useTexture");
	locationLightColor = glGetUniformLocation(program[0], "lightColor");
	locationLightEnabled = glGetUniformLocation(program[0], "lightEnabled");
	locationMagentaEnabled = glGetUniformLocation(program[0], "magentaEnabled");
	/** Fekete lesz a háttér. */
	/** Background is black. */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	/** Z-buffer engedélyezése. */
	/** Enable Z coordinate check for visibility. */
	glEnable(GL_DEPTH_TEST);
	/** Ha vonalakat rajzolunk, módosítsuk a vastagságát! */
	/** When drawing lines, we can make experiments with different widths! */
	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		lightEnabled = !lightEnabled;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		magentaEnabled = !magentaEnabled;
	}
}

void computeCameraMatrix() {
	cameraPosition = vec3(
		cameraRadius * cos(cameraAngle),
		cameraHeight,
		cameraRadius * sin(cameraAngle)
	);

	matView = lookAt(cameraPosition, cameraTarget, cameraUpVector);
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	static double lastFrame = 0;
	double dt = currentTime - lastFrame;
	lastFrame = currentTime;

	float speed = 1.5f * dt;

	if (keyboard[GLFW_KEY_A] || keyboard[GLFW_KEY_LEFT]) cameraAngle -= speed;
	if (keyboard[GLFW_KEY_D] || keyboard[GLFW_KEY_RIGHT]) cameraAngle += speed;
	if (keyboard[GLFW_KEY_W] || keyboard[GLFW_KEY_UP]) cameraHeight += speed * 2.0f;
	if (keyboard[GLFW_KEY_S] || keyboard[GLFW_KEY_DOWN]) cameraHeight -= speed * 2.0f;

	computeCameraMatrix();
	glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));

	lightAngle += dt;

	lightPosition = vec3(
		lightRadius * cos(lightAngle),
		2.0f,
		lightRadius * sin(lightAngle)
	);

	glUniform3fv(locationLightPos, 1, value_ptr(lightPosition));
	glUniform3fv(locationLightColor, 1, value_ptr(lightColor));
	glUniform1i(locationLightEnabled, lightEnabled);
	glUniform1i(locationMagentaEnabled, magentaEnabled);

	float side = 1.0f;

	vec3 positions[3] = {
		vec3(-side * 2.0f, 0, 0),
		vec3(0, 0, 0),
		vec3(side * 2.0f, 0, 0)
	};

	for (int i = 0; i < 3; i++) {
		glBindVertexArray(VAO[VAOVerticesData]);

		mat4 model = translate(mat4(1.0f), positions[i]);
		glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(model));

		glUniform1i(locationUseTexture, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glDrawElements(GL_TRIANGLES, cube_indices.size(), GL_UNSIGNED_INT, nullptr);
	}

	if (showLightSphere)
	{
		glBindVertexArray(VAO[VAOSun]);

		mat4 sunModel = translate(mat4(1.0f), lightPosition);
		sunModel = scale(sunModel, vec3(0.25f));

		glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(sunModel));

		glUniform1i(locationUseTexture, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sunTexture);

		glDrawElements(
			GL_TRIANGLES,
			(GLsizei)sphere_indices.size(),
			GL_UNSIGNED_INT,
			0
		);
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	/** A minimalizálás nem fog fagyni a minimum 1 értékkel. */
	/** Minimize will not freeze with minimum value 1. */
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);

	GLfloat aspectRatio = (GLfloat)windowWidth / (GLfloat)windowHeight;
	/** A kezelt képernyõ beállítása a teljes (0, 0, width, height) területre. */
	/** Set the viewport for the full (0, 0, width, height) area. */
	glViewport(0, 0, windowWidth, windowHeight);
	matProjection = perspective(
		radians(45.0f),	// The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		aspectRatio,	// Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar?
		0.1f,			// Near clipping plane. Keep as big as possible, or you'll get precision issues.
		100.0f			// Far clipping plane. Keep as little as possible.
	);
	/** Uniform változók beállítása. */
	/** Setup uniform variables. */
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (firstMouse) {
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	float dx = float(xPos - lastX);
	float dy = float(yPos - lastY);

	lastX = xPos;
	lastY = yPos;

	float sensitivity = 0.005f;

	cameraAngle += dx * sensitivity;
	cameraHeight += dy * sensitivity;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
}

int main(void) {
	init(3, 3, GLFW_OPENGL_CORE_PROFILE);
	initShaderProgram();
	initTextures();
	framebufferSizeCallback(window, windowWidth, windowHeight);
	setlocale(LC_ALL, "hu_HU");

	cout << "ESC\t\texit" << endl;
	cout << "L\t\tinduces lighting toggle" << endl;
	cout << "M\t\tinduces magenta material toggle" << endl;
	cout << "WASD, arrows and mouse movement\tcamera control" << endl << endl;
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUpScene(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}