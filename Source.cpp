
#include <iostream>
#include <gl/glew/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <gl/glm/glm.hpp>
#include<gl\glm\gtc\type_ptr.hpp>
#include<gl\glm\gtx\transform.hpp>
#include <noise/noise.h>

using namespace std;
using namespace noise;

struct vec3
{
	float x, y, z;
};

struct vertex
{
	vec3 position;
	vec3 color;
};

const GLint WIDTH = 600, HEIGHT = 600;
GLuint VAO, VBO, programId;
GLuint InitShader(const char* vertex_shader_file_name, const char* fragment_shader_file_name);
GLuint modMatLocation, viewMatLoc, projMatLoc;
module::Perlin moiseModule;
float theta = -90;
float animate = 0;
int cols = 40;
int rows = 40;

enum DrawingMode
{
	Points,
	Lines,
	Lines_Strip,
	FilledTriangle,
	OutlinedTriangle
};
DrawingMode current_mode = DrawingMode::OutlinedTriangle;

vector<vector<vertex>> vertices;
vector<vec3> randomColors;

void CreateTriangle()
{
	vertices.clear();
	vertices.push_back({ {-1,-1,0}, {0,1,0} });
	vertices.push_back({ {1,-1,0}, {0,0,1} });
	vertices.push_back({ {0,1,0}, {1,0,0} });

	// create VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vec3), ((char*)(3 * sizeof(vec3))));
	glEnableVertexAttribArray(1);

}


void GenerateRandomColors()
{
	randomColors.clear();
	for (float y = 0 - rows; y < rows; y++)
	{
		for (float x = 0 - cols; x <= cols; x++)
		{
			randomColors.push_back({ (float)(rand() % 255) / 255.0f, (float)(rand() % 255) / 255.0f, (float)(rand() % 255) / 255.0f });
		}
	}
}

int scl = 30;
void CreateTrain()
{
	vertices.clear();
	float interval = 0.1f;
	float xoffset = 0;
	float yoffset = animate;
	float zoffset = 0.5;

	vector<vertex> row;
	int index = 0;

	for (float y = 0 - rows; y < rows; y++)
	{
		row.clear();
		for (float x = 0 - cols; x <= cols; x++)
		{
			vec3 v = { x / scl,y / scl, (float)moiseModule.GetValue(xoffset, yoffset, zoffset) };
			row.push_back({ v ,randomColors[index] });
			v = { x / scl,(y + 1) / scl , (float)moiseModule.GetValue(xoffset, yoffset, zoffset) };
			index++;
			if (index >= randomColors.size())
				index = 0;
			row.push_back({ v ,randomColors[index] });

			xoffset += 0.0001f;
		}
		vertices.push_back(row);
		yoffset += 0.0001f;

	}
}

void CompileShader()
{
	programId = InitShader("VS.glsl", "FS.glsl");
	glUseProgram(programId);
}

int Init()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cout << "Error";
		getchar();
		return 1;
	}
	else
	{
		if (GLEW_VERSION_3_0)
			cout << "Driver support OpenGL 3.0\nDetails:\n";
	}
	cout << "\tUsing glew " << glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	glGenBuffers(1, &VBO);
	CompileShader();
	GenerateRandomColors();

	modMatLocation = glGetUniformLocation(programId, "modMat");
	viewMatLoc = glGetUniformLocation(programId, "viewMat");
	projMatLoc = glGetUniformLocation(programId, "projMat");

	glm::mat4 viewMat = glm::lookAt(glm::vec3(0, 2, -2.7), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 projMat = glm::perspectiveFov(45.f, 5.0f, 5.0f, 0.1f, 100.0f);

	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));


	glClearColor(0, 0, 0, 1);

	return 0;

}


void Update()
{
	animate += 0.001;
	/*theta += 0.001f;
	GLuint thetaUniformLocation = glGetUniformLocation(programId, "theta");
	glUniform1f(thetaUniformLocation, theta);*/
}

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	CreateTrain();

	glm::mat4 ModelMat = glm::rotate(theta * 180 / 3.14f, glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modMatLocation, 1, GL_FALSE, glm::value_ptr(ModelMat));

	for (int i = 0; i < vertices.size(); i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(vertex), vertices[i].data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vertex), ((char*)(sizeof(vec3))));
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices[i].size());
	}
}

int main()
{
	sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "Flaying Mat!");

	if (Init()) return 1;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{

			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		Update();
		Render();

		window.display();
	}
	return 0;
}