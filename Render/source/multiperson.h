#pragma once
#include <fstream>
#include <algorithm>
#include <string>
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <string>
#include "Renderer.h"
#include "background.h"
#include "functions.h"
#include "cal_smpl.h"

void LoadHand(vector<bool>& HandJudger, string HandFilename);

void MultiInit(Renderer& mesh, int num, const char* vs_name, const char* fs_name, string mesh_filename, string sample_filename, string HandFilename);

void MultiSetUniform(Renderer& mesh, int num, const int RenderDepth);

void GenPosition(float& angle1, float& angle2, glm::vec3& trans, glm::vec3& transold, int seed);

void GenPosition(float& angle1, float& angle2, glm::vec3& trans, int seed);

void GetBoundingBox(int num, string BoundingBox_filename);

void GetProjection(int num);

void Cal_Joints(string in_filename, string out_filename, glm::mat4 model, glm::mat4 view, glm::mat4 projection);

void MultiDisplay(background& text, GLFWwindow* window, const char* vs_name, const char* fs_name);
