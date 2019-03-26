#include "Renderer.h"
#include "LoadShaders.h"
#include <iostream>
using namespace std;

Renderer::Renderer()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(3, vbo_array_buffer);
    glGenBuffers(1, &vbo_element_array_buffer);         
    data_seted = false;
    color_seted = false;
}

Renderer::~Renderer()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(3, vbo_array_buffer);
    glDeleteBuffers(1, &vbo_element_array_buffer);
}

void Renderer::Init(const string vs_name, const string fs_name)
{
    //add shaders
    ShaderInfo shader_info[] =
    {
        { GL_VERTEX_SHADER, vs_name.c_str() },
        { GL_FRAGMENT_SHADER, fs_name.c_str() },
        { GL_NONE, NULL }
    };
    prog = LoadShaders(shader_info);
}

void Renderer::CalNorm_SetBuffer(const vector<glm::vec3>& vertex)
{
	//计算各点“法向”
    vector<glm::vec3> normal(vertex_count, glm::vec3(0, 0, 0));
    for (unsigned int f = 0; f < face_count; f++)
    {
        glm::ivec3 f_id(face[f]);
        glm::vec3 vx(vertex[f_id.x]);
        glm::vec3 vy(vertex[f_id.y]);
        glm::vec3 vz(vertex[f_id.z]);
        glm::vec3 n = glm::cross(vy - vx, vz - vx);
        normal[f_id.x] += n;
        normal[f_id.y] += n;
        normal[f_id.z] += n;
    }
    for (unsigned int v = 0; v < vertex_count; v++)
    {
        normal[v] = glm::normalize(normal[v]);
    }

	Normal = normal;//赋值法向vector

    vector<glm::vec3> vertex_face(3 * face_count);
    vector<glm::vec3> normal_face(3 * face_count);
    for (unsigned int f = 0; f < face_count; f++)
    {
        glm::ivec3 f_id(face[f]);
        vertex_face[3 * f] = vertex[f_id.x];
        vertex_face[3 * f + 1] = vertex[f_id.y];
        vertex_face[3 * f + 2] = vertex[f_id.z];
        normal_face[3 * f] = normal[f_id.x];
        normal_face[3 * f + 1] = normal[f_id.y];
        normal_face[3 * f + 2] = normal[f_id.z];
    }

    glBindVertexArray(vao);

    if (!data_seted)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_array_buffer[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * face_count * sizeof(glm::vec3), &vertex_face[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        //glNamedBufferStorage(vbo_array_buffer[0], 3 * face_count * sizeof(glm::vec3), vertex_face.data(), GL_DYNAMIC_STORAGE_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_array_buffer[1]);
		glBufferData(GL_ARRAY_BUFFER, 3 * face_count * sizeof(glm::vec3), &normal_face[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        //glNamedBufferStorage(vbo_array_buffer[1], 3 * face_count * sizeof(glm::vec3), normal_face.data(), GL_DYNAMIC_STORAGE_BIT);
    }
	/*
    else
    {
        glNamedBufferSubData(vbo_array_buffer[0], 0, 3 * face_count * sizeof(glm::vec3), vertex_face.data());
        glNamedBufferSubData(vbo_array_buffer[1], 0, 3 * face_count * sizeof(glm::vec3), normal_face.data());
    }
	*/

    glBindVertexArray(0);

    data_seted = true;
}

void Renderer::SetData(const vector<glm::vec3>& vertex_mesh, const vector<glm::ivec3>& edge_mesh)
{
    //set data

    vertex_count = (unsigned int)vertex_mesh.size();
    face = edge_mesh;
    face_count = (unsigned int)face.size();

    CalNorm_SetBuffer(vertex_mesh);
}

void Renderer::SetColor(const vector<glm::vec3>& color, const bool for_face)
{
    if (face_count == 0)
    {
        cerr << "Error in function SetColor: face is not seted\n"; return;
    }

    vector<glm::vec3> color_face;
    if (for_face)
    {
        color_face = color;
    }
    else
    {
        color_face = vector<glm::vec3>(3 * face_count);
        for (unsigned int f = 0; f < face_count; f++)
        {
            glm::ivec3 f_id(face[f]);
            color_face[3 * f] = color[f_id.x];
            color_face[3 * f + 1] = color[f_id.y];
            color_face[3 * f + 2] = color[f_id.z];
        }
    }
    glBindVertexArray(vao);
    if (!color_seted)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_array_buffer[2]);
		glBufferData(GL_ARRAY_BUFFER, 3 * face_count * sizeof(glm::vec3), &color_face[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
        //glNamedBufferStorage(vbo_array_buffer[2], 3 * face_count * sizeof(glm::vec3), color_face.data(), GL_DYNAMIC_STORAGE_BIT);
    }
	/*
    else
    {
        glNamedBufferSubData(vbo_array_buffer[2], 0, 3 * face_count * sizeof(glm::vec3), color_face.data());
    }
	*/
    glBindVertexArray(0);
    color_seted = true;
}

void Renderer::SetUniforms(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                           const int render_depth)
{
    glUseProgram(prog);

    GLuint model_matrix_loc = glGetUniformLocation(prog, "model_matrix");
    GLuint view_matrix_loc = glGetUniformLocation(prog, "view_matrix");
    GLuint projection_matrix_loc = glGetUniformLocation(prog, "projection_matrix");
    GLuint render_depth_loc = glGetUniformLocation(prog, "render_depth");

    glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(render_depth_loc, render_depth);

    glUseProgram(0);
}

void Renderer::SetLight()
{
	glUseProgram(prog);

	GLint lightColorLoc = glGetUniformLocation(prog, "lightColor");
	GLint lightPosLoc = glGetUniformLocation(prog, "lightPos");
	GLint viewPosLoc = glGetUniformLocation(prog, "viewPos");

	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc, 1.0f, 0.0f, 0.0f);
	glUniform3f(viewPosLoc, viewPosition.x, viewPosition.y, viewPosition.z);

	glUseProgram(0);
}

void Renderer::Draw(const bool draw_point)
{
    glUseProgram(prog);
    glBindVertexArray(vao);
    if (!draw_point)
    {
        glDrawArrays(GL_TRIANGLES, 0, 3 * face_count);
    }
    else
    {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, 3 * vertex_count);
    }
    glUseProgram(0);
    glBindVertexArray(0);
}
