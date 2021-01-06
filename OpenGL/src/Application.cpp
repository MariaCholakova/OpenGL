#include "Error.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource {
	std::string vertexSource;
	std::string fragmentSource;
};

// read shaders source from file
static ShaderProgramSource parseShader(const std::string& filepath) {
	std::ifstream stream(filepath);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line)) {
		// the first line of the shader file must be #shader .... otherwise type will be -1
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			ss[(int)type] << line << "\n";
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int compileShader(unsigned int type, const std::string& source)
{
	GLCall(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		// shader is not compiled successfully
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

// create shader program and attach vertex and fragment shaders to it
static unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	GLCall(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Set the swap interval for the current context, i.e. the number of screen updates to wait
	from the time glfwSwapBuffers was called before swapping the buffers and returning */
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "glew init error" << std::endl;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	{
		float positions[] = {
			-0.5f, -0.5f,
			 0.5f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f,
		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		/* Generate a vertex array object and bind it */
		unsigned int vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		/* Generate a buffer array object and bind it */
		VertexBuffer vb(positions, 4 * 2 * sizeof(float));

		/* Link the buffer to the vao */
		GLCall(glEnableVertexAttribArray(0));
		/* Set index 0 of the currently bound vertex array (vao) to point to the currently bound GL_ARRAY_BUFFER (buffer) */
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

		IndexBuffer ib(indices, 6);

		ShaderProgramSource source = parseShader("resources/shaders/Basic.shader");
		std::cout << "VERTEX" << std::endl;
		std::cout << source.vertexSource << std::endl;
		std::cout << "FRAGMENT" << std::endl;
		std::cout << source.fragmentSource << std::endl;

		unsigned int shader = createShader(source.vertexSource, source.fragmentSource);
		GLCall(glUseProgram(shader));

		/* Retrieve the location of the u_Color variable from the shader we just bound */
		GLCall(int uColorLocation = glGetUniformLocation(shader, "u_Color"));
		ASSERT(uColorLocation != -1);

		/* Unbind everything - clear the state */
		GLCall(glBindVertexArray(0));
		GLCall(glUseProgram(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		float red = 0.0f;
		float increment = 0.05f;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			GLCall(glUseProgram(shader));
			/* Uniforms are set per draw, i.e. before a draw call! */
			GLCall(glUniform4f(uColorLocation, red, 0.3f, 0.8f, 1.0f));

			GLCall(glBindVertexArray(vao));
			ib.Bind();

			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

			/* Change the red color */
			if (red > 1.0f)
				increment = -0.05f;
			else if (red < 0.0f)
				increment = 0.05f;
			red += increment;

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		GLCall(glDeleteProgram(shader));
	}

	glfwTerminate();
	return 0;
}