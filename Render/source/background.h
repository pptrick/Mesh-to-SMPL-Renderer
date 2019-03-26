#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class background
{
private:
	GLuint textVAO;
	GLuint textVBO;
	GLuint textEBO;
	GLuint background_text;
	vector<GLfloat>vertices;
	vector<GLint>indices;
	
public:
	background();
	~background();
	GLuint textprog;
	void Init(const string vs_name, const string fs_name, const char *filename);
	void loaddata();
	void adjust_view();
	void Bindbuffer();
	void Gentexture(const char *filename);
	void Drawtexture();
};
