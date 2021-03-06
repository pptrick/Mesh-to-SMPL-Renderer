#include <fstream>
#include <string>
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "Renderer.h"
#include "background.h"
#include "functions.h"
#include "cal_smpl.h"
#include "multiperson.h"

using namespace std;
double xpos, ypos;
double viewx = 1.5;
double viewy = 0;
double viewz = 0;
float angle1 = 90.0f;
float angle2 = 0;
float nearz = 0.5f;
float farz = 6.0f;
bool closeloop = 0;
bool use_background = 1;
float view_dist = 1.3;

glm::mat4 model = glm::mat4(1.0);
glm::mat4 model_old = glm::mat4(1.0);
glm::mat4 view = glm::mat4(1.0);
glm::mat4 projection = glm::mat4(1.0);
glm::vec3 viewpoint = glm::vec3(0.0f, 0.0f, 0.0f);


#define WIDTH 800
#define HEIGHT 600

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwGetCursorPos(window, &xpos, &ypos);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		model_old = model; 
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		angle1 += 5.f;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		angle1 -= 5.f;
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		angle2 += 5.f;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		angle2 -= 5.f;
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		viewpoint.y += 0.3f;
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		viewpoint.y -= 0.3f;
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		viewpoint.x += 0.3f;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		viewpoint.x -= 0.3f;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		view_dist -= 0.07f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		view_dist += 0.07f;
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		closeloop = 1;
}

void SetViewProjection(glm::mat4& view, glm::mat4& projection, GLFWwindow* window, const glm::vec3 center)
{
	double xpos_new, ypos_new;
	const double radius = 500;
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		cout << "detect axis" << endl;
		glfwGetCursorPos(window, &xpos_new, &ypos_new);
		double x = xpos - WIDTH / 2, y = ypos - HEIGHT / 2;
		double xnew = xpos_new - WIDTH / 2, ynew = ypos_new - HEIGHT / 2;
		x /= radius; y /= radius; xnew /= radius; ynew /= radius;
		double z = sqrt(1 - x * x - y * y);
		double znew = sqrt(1 - xnew * xnew - ynew * ynew);
		Eigen::Vector3f p((float)x, (float)y, (float)z);
		Eigen::Vector3f pnew((float)xnew, (float)ynew, (float)znew);
		Eigen::Vector3f axis = p.cross(pnew);
		if (axis.norm() > 1e-4)
		{
			float theta = 2 * asinf((p - pnew).norm() / 2);
			glm::mat4 rot = glm::rotate(glm::mat4(1.0), theta , glm::vec3(-axis(0), -axis(1), axis(2)));
			glm::mat4 trans1 = glm::translate(glm::mat4(1.0), center);
			glm::mat4 trans2 = glm::translate(glm::mat4(1.0), -center);
			model = trans1 * rot * trans2 * model_old;
		}
	}
	float aspect = (float)HEIGHT / (float)WIDTH;
	view = glm::mat4(glm::lookAt(center - glm::vec3(viewx, viewy, viewz),center - viewpoint,glm::vec3(0.0f, -1.0f, 0.0f)));
	projection = glm::mat4(glm::perspective(glm::radians(90.0f), 1.0f / aspect, nearz, farz));
}

void Display(Renderer& mesh, background& text,GLFWwindow* window, const glm::vec3 center, const vector<glm::vec3>& sample_vertex,
	const string color_filename, const string corr_filename,
	const string depth_filename, const string front_obj_filename)
{
	static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, black);

	//SetViewProjection(view, projection, window, center);

	cv::Mat render_results(HEIGHT, WIDTH, CV_8UC3);
	if(use_background)text.Drawtexture();
	mesh.SetUniforms(model, view, projection, 0);
	mesh.Draw();
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, render_results.data);
	cv::flip(render_results, render_results, 0);
	cv::imwrite(color_filename, render_results);

	glfwSwapBuffers(window);
	glfwPollEvents();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, black);

	mesh.SetUniforms(model, view, projection, 1);
	mesh.Draw();
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, render_results.data);
	cv::flip(render_results, render_results, 0);
	cv::imwrite(depth_filename, render_results);

	ofstream fout_obj(front_obj_filename.c_str());
	ofstream fout_corr(corr_filename.c_str());
	{
		cv::Mat show_points(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));
		glm::mat4 mvp = projection * view * model;
		for (int i = 0; i < sample_vertex.size(); i++)
		{
			fout_obj << "v " << sample_vertex[i].x << ' ' << sample_vertex[i].y << ' ' << sample_vertex[i].z << ' ';
			glm::vec4 proj = mvp * glm::vec4(sample_vertex[i], 1.0f);
			proj = proj * (1.0f / proj.w);
			int u = (int)roundf(WIDTH * (proj.x / 2 + 0.5f));
			int v = (int)roundf(HEIGHT * (-proj.y / 2 + 0.5f));
			float depth_color = 255 * (proj.z * 0.5f + 0.5f);
			if (v >= 0 && v < HEIGHT && u >= 0 && u < WIDTH &&
				abs(depth_color - (float)render_results.at<cv::Vec3b>(v, u)[0]) < 0.5)
			{
				show_points.at<uchar>(v, u) = (int)roundf(255 * proj.z);
				fout_obj << "0.0 1.0 0.0" << endl;
				fout_corr << i << ' ' << u << ' ' << v << endl;
			}
			else
			{
				fout_obj << "1.0 0.0 0.0" << endl;
			}
		}
	}
	fout_obj.close();
	fout_corr.close();
}

void cal_angle()
{
	double old_x, old_y, old_z;
	old_x = viewx;
	old_y = viewy;
	old_z = viewz;
	viewx = view_dist * sin(glm::radians(angle1))*sin(glm::radians(angle2));
	viewy = view_dist * cos(glm::radians(angle1));
	viewz = view_dist * sin(glm::radians(angle1))*cos(glm::radians(angle2));
	if (old_x != viewx || old_y != viewy || old_z != viewz)
		cout << "angle update: angle1= " << angle1 << " angle2= " << angle2 << " view distance= " << view_dist << endl;
}

void Cal_Joints(string in_filename, string out_filename,glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	//Load Joints:
	glm::vec4 Joints[24];
	ifstream fin(in_filename);
	ofstream fout(out_filename);
	for (int i = 0; i < 24; i++)
	{
		fin >> Joints[i].x;
		fin >> Joints[i].y;
		fin >> Joints[i].z;
		Joints[i].w = 1.0f;
		//transform:
		Joints[i] = projection * view * model * Joints[i];
		//standard:
		Joints[i].x = Joints[i].x / Joints[i].w;
		Joints[i].y = Joints[i].y / Joints[i].w;
		Joints[i].z = Joints[i].z / Joints[i].w;
		Joints[i].w = 1.0f;
		fout << (WIDTH/2) * (Joints[i].x+1) << " " << (HEIGHT/2) * (-Joints[i].y+1) << endl;
	}
}

//显示坐标函数
/*
void show()
{
	ifstream fin("../output/joints_coor1.txt");
	int u, v;
	double x, y;
	cv::Mat show_points(HEIGHT, WIDTH, CV_8UC3);

	while (fin >> x)
	{
		fin >> y;
		u = (int)x;
		v = (int)y;
		show_points.at<cv::Vec3b>(u, v)[0] = 255;
	}

	cv::namedWindow("show");
	cv::imshow("show", show_points);
	cv::waitKey(0);
}
*/

int main()
{
	//clock_t t = clock();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "render", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	
	glewExperimental = GL_TRUE;
	glewInit();

	cout << "OpenGL version:" << glGetString(GL_VERSION) << endl;

	glfwSetKeyCallback(window, key_callback);
	glViewport(0, 0, WIDTH, HEIGHT);

	background text;
	text.Init("../glsl/vs_text.glsl", "../glsl/fs_text.glsl", "../data/moon.jpg");

	MultiDisplay(text, window, "../glsl/vs_color.glsl", "../glsl/fs_color.glsl");

	//show();

	glfwDestroyWindow(window);
	glfwTerminate();
	//t = clock() - t;
	//std::cout << "time: " << (float(t) / CLOCKS_PER_SEC) << "seconds\n";

	return 0;
}


//速度优化思路：耗时主要在MultiDisplay