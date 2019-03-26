#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
//#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Renderer 
{
public:
    Renderer();
    ~Renderer();

    void Init(const string vs_name, const string fs_name);
    void SetData(const vector<glm::vec3>& vertex_mesh, const vector<glm::ivec3>& edge_mesh);
    void SetColor(const vector<glm::vec3>& color, const bool for_face = false);
    void SetUniforms(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                     const int render_depth);
	void SetLight();
    void Draw(const bool draw_point = false);
	
	GLuint vbo_array_buffer[3];
	GLuint vbo_element_array_buffer;
	GLuint vao;
	GLuint prog;

	glm::vec3 viewPosition;
	unsigned int face_count;

private:
    unsigned int vertex_count;
    vector<glm::ivec3> face;
	vector<glm::vec3> Normal;

    void CalNorm_SetBuffer(const vector<glm::vec3>& vertex);
    bool data_seted;
    bool color_seted;
};
