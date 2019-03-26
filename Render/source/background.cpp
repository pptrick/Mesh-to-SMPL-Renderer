#include "background.h"
#include "LoadShaders.h"
#include <iostream>

using namespace std;

#define WIDTH 800
#define HEIGHT 600

extern double viewx;
extern double viewy;
extern double viewz;
extern float nearz;
extern float farz;

background::background()
{
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glGenBuffers(1, &textEBO);
}

background::~background()
{
	glDeleteVertexArrays(1, &textVAO);
	glDeleteBuffers(1, &textVBO);
	glDeleteBuffers(1, &textEBO);
}

void background::Init(const string vs_name, const string fs_name, const char *filename)
{
	ShaderInfo shader_info[] =
	{
		{ GL_VERTEX_SHADER, vs_name.c_str() },
	{ GL_FRAGMENT_SHADER, fs_name.c_str() },
	{ GL_NONE, NULL }
	};
	textprog = LoadShaders(shader_info);

	loaddata();
	Bindbuffer();
	Gentexture(filename);
}

void background::loaddata()
{
	float dep = 0.999;
	GLfloat vertices_buffer[] =
	{
		-1.0f,-1.0f,dep,    0.0f,0.0f,
		1.0f,-1.0f,dep,     1.0f,0.0f,
		-1.0f,1.0f,dep,     0.0f,1.0f,
		1.0f,1.0f,dep,      1.0f,1.0f
	};

	GLint indices_buffer[] =
	{
		0,1,2,
		1,2,3
	};

	for (int i = 0; i < 20; i++) vertices.push_back(vertices_buffer[i]);

	for (int i = 0; i < 6; i++) indices.push_back(indices_buffer[i]);
	
}

void background::adjust_view()
{
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 view = glm::mat4(1.0);
	glm::mat4 projection = glm::mat4(1.0);

	GLuint model_matrix_loc = glGetUniformLocation(textprog, "model");
	GLuint view_matrix_loc = glGetUniformLocation(textprog, "view");
	GLuint projection_matrix_loc = glGetUniformLocation(textprog, "projection");

	glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(Model));
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection));
}

void background::Bindbuffer()
{
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void background::Gentexture(const char *filename)
{
	glGenTextures(1, &background_text);

	glBindTexture(GL_TEXTURE_2D, background_text);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cv::Mat img = cv::imread(filename);
	int Width = img.cols;
	int Height = img.rows;
	int channel = img.channels();

	int pixellength = Width * Height * channel;
	void *pixels = new GLubyte[pixellength];
	memcpy(pixels, img.data, pixellength * sizeof(char));

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void background::Drawtexture()
{
	adjust_view();

	glBindTexture(GL_TEXTURE_2D, background_text);
	glUseProgram(textprog);

	glBindVertexArray(textVAO);
	glDrawElements(GL_TRIANGLES, sizeof(GLfloat) * 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}