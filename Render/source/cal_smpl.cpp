#include "cal_smpl.h"
#include "LoadShaders.h"
#include <iostream>

using namespace std;

#define WIDTH 800
#define HEIGHT 600

bool mesh_or_simple = 1;


smpl::smpl()
{
	glGenVertexArrays(1, &smplVAO);
	glGenBuffers(1, &smplVBO);
	glGenBuffers(1, &smplEBO);
}

smpl::~smpl()
{
	glDeleteVertexArrays(1, &smplVAO);
	glDeleteBuffers(1, &smplVBO);
	glDeleteBuffers(1, &smplEBO);
}

void smpl::Init(const string vs_name, const string fs_name, string smpl_filename, const char* seg_filename, string sample_filename, vector<bool> HandJudger)
{
	ShaderInfo shader_info[] =
	{
		{ GL_VERTEX_SHADER, vs_name.c_str() },
	{ GL_FRAGMENT_SHADER, fs_name.c_str() },
	{ GL_NONE, NULL }
	};
	smplprog = LoadShaders(shader_info);
	load_smpl_data(smpl_filename);
	load_segment_data(seg_filename);
	load_sample_data(sample_filename);
	cal_color(HandJudger);
	Bindbuffer();
}

void smpl::Bindbuffer()
{
	glBindVertexArray(smplVAO);
	glBindBuffer(GL_ARRAY_BUFFER, smplVBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, smplEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLint), &faces[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void smpl::SetUniforms(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
	glUseProgram(smplprog);

	GLuint model_matrix_loc = glGetUniformLocation(smplprog, "model_matrix");
	GLuint view_matrix_loc = glGetUniformLocation(smplprog, "view_matrix");
	GLuint projection_matrix_loc = glGetUniformLocation(smplprog, "projection_matrix");

	glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection));

	glUseProgram(0);
}

void smpl::display(GLFWwindow* window,const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const string segment_filename)
{
	glfwPollEvents();
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, black);

	cv::Mat segment_results(HEIGHT, WIDTH, CV_8UC3);
	SetUniforms(model, view, projection);
	
	glUseProgram(smplprog);
	glBindVertexArray(smplVAO);
	glDrawElements(GL_TRIANGLES, sizeof(GLfloat) * faces.size(), GL_UNSIGNED_INT, 0);
	glUseProgram(0);
	glBindVertexArray(0);

	glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, segment_results.data);
	cv::flip(segment_results, segment_results, 0);
	cv::imwrite(segment_filename, segment_results);

	glfwSwapBuffers(window);
}

void smpl::MultiSegDraw(GLFWwindow* window, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection)
{
	SetUniforms(model, view, projection);

	glUseProgram(smplprog);
	glBindVertexArray(smplVAO);
	glDrawElements(GL_TRIANGLES, sizeof(GLfloat) * faces.size(), GL_UNSIGNED_INT, 0);
	glUseProgram(0);
	glBindVertexArray(0);

}



void smpl::load_smpl_data(string filename)
{
	ifstream fin(filename);
	char buffer;
	glm::vec3 coor;
	glm::vec3 num;
	points.clear();
	faces.clear();
	while (fin >> buffer)
	{
		if (buffer != 'v')break;
		fin >> coor.x >> coor.y >> coor.z;
		points.push_back(coor.x);
		points.push_back(coor.y);
		points.push_back(coor.z);
	}
	if (buffer == 'f')
	{
		do
		{
			fin >> num.x >> num.y >> num.z;
			num.x -= 1;
			num.y -= 1;
			num.z -= 1;
			faces.push_back(num.x);
			faces.push_back(num.y);
			faces.push_back(num.z);
		} while (fin >> buffer);
	}
	fin.close();
}


void smpl::load_segment_data(string filename)
{
	ifstream fin(filename);
	float buffer;
	int seg_num;
	segmentation.clear();
	while (fin>>buffer)
	{
		fin >> seg_num;
		segmentation.push_back(seg_num);
		for (int i = 0; i < 4; i++) fin >> buffer;
	}
	fin.close();
}

void smpl::load_sample_data(string filename)
{
	ifstream fin(filename);
	char buffer;
	glm::vec3 coor;
	sample_vertex.clear();
	while (fin >> buffer)
	{
		if (buffer != 'v')break;
		fin >> coor.x >> coor.y >> coor.z;
		sample_vertex.push_back(coor.x);
		sample_vertex.push_back(coor.y);
		sample_vertex.push_back(coor.z);
	}
	fin.close();
}

void smpl::cal_color(vector<bool> HandJudger)
{
	int num;
	glm::vec3 color;
	if (segmentation.size() != points.size() / 3 || segmentation.size() != sample_vertex.size() / 3 || segmentation.size() != HandJudger.size())
	{
		cout << "anotated points number error!" << endl;
		return;
	}
	for (int i = 0; i < segmentation.size(); i++)
	{
		//if (HandJudger[i] == 0)continue;
		num = segmentation[i];
		switch (num)
		{
		case 1:
			color = glm::vec3(1.0, 0, 0);
			break;
		case 2:
			color = glm::vec3(0.8, 0.2, 0);
			break;
		case 3:
			color = glm::vec3(0.6, 0.4, 0);
			break;
		case 4:
			color = glm::vec3(0.4, 0.6, 0);
			break;
		case 5:
			color = glm::vec3(0.2, 0.8, 0);
			break;
		case 6:
			color = glm::vec3(0, 1.0, 0);
			break;
		case 7:
			color = glm::vec3(0, 0.8, 0.2);
			break;
		case 8:
			color = glm::vec3(0, 0.6, 0.4);
			break;
		case 9:
			color = glm::vec3(0, 0.4, 0.6);
			break;
		case 10:
			color = glm::vec3(0, 0.2, 0.8);
			break;
		case 11:
			color = glm::vec3(0, 0, 1.0);
			break;
		case 12:
			color = glm::vec3(0.2, 0, 0.8);
			break;
		case 13:
			color = glm::vec3(0.4, 0, 0.6);
			break;
		case 14:
			color = glm::vec3(0.6, 0, 0.4);
			break;
		case 15:
			color = glm::vec3(0.8, 0, 0.2);
			break;
		case 16:
			color = glm::vec3(0.3, 0.4, 0.3);
			break;
		case 17:
			color = glm::vec3(0.3, 0.6, 0.3);
			break;
		case 18:
			color = glm::vec3(0.8, 0.6, 0.4);
			break;
		case 19:
			color = glm::vec3(0.5, 0.7, 0.2);
			break;
		case 20:
			color = glm::vec3(0.1, 0.3, 0.6);
			break;
		case 21:
			color = glm::vec3(0.3, 0.2, 0.8);
			break;
		case 22:
			color = glm::vec3(0.2, 0.9, 0.4);
			break;
		case 23:
			color = glm::vec3(0.5, 0.8, 0.1);
			break;
		case 24:
			color = glm::vec3(0.9, 0.9, 0.3);
			break;
		default:
			color = glm::vec3(0, 0, 0);
			break;
		}
		if (mesh_or_simple)
		{
			data.push_back(sample_vertex[3 * i]);
			data.push_back(sample_vertex[3 * i + 1]);
			data.push_back(sample_vertex[3 * i + 2]);
			data.push_back(color.x);
			data.push_back(color.y);
			data.push_back(color.z);
		}
		else
		{
			data.push_back(points[3 * i]);
			data.push_back(points[3 * i + 1]);
			data.push_back(points[3 * i + 2]);
			data.push_back(color.x);
			data.push_back(color.y);
			data.push_back(color.z);
		}
	}
}