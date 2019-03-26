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

class smpl
{
private:
	vector<float>points;
	vector<float>data;
	vector<int>faces;
	vector<int>segmentation;
	vector<float>sample_vertex;
	GLuint smplVAO;
	GLuint smplVBO;
	GLuint smplEBO;
public:
	smpl();
	~smpl();
	GLuint smplprog;
	void Init(const string vs_name, const string fs_name, string smpl_filename, const char* seg_filename, string sample_filename, vector<bool> HandJudger);
	void Bindbuffer();
	void SetUniforms(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	void display(GLFWwindow* window, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const string segment_filename);
	void MultiSegDraw(GLFWwindow* window, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	void load_segment_data(string filename);
	void load_smpl_data(string filename);
	void load_sample_data(string filename);
	void cal_color(vector<bool> HandJudger);
};
