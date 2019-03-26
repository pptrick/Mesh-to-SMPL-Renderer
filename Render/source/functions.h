#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <string>
using namespace std;

static void ObjReader(vector<glm::vec3>& vertex, vector<glm::ivec3>& edge,
                      const string obj_filename)
{
    ifstream fin(obj_filename.c_str());
    string c;
    vertex.resize(0);
    edge.resize(0);
    while (fin >> c)
    {
        if (c == "v")
        {
            float vx, vy, vz; fin >> vx >> vy >> vz;
            vertex.push_back(glm::vec3(vx, vy, vz));
        }
        else if (c == "f")
        {
            int idx, idy, idz; fin >> idx >> idy >> idz;
            edge.push_back(glm::ivec3(idx - 1, idy - 1, idz - 1));
        }
    }
    fin.close();
}

static void ObjReader(vector<glm::vec3>& vertex, vector<glm::ivec3>& edge, vector<glm::vec3>& color,
                      const string obj_filename, const int fnum = 1)
{
	std::ios::sync_with_stdio(false);
    ifstream fin(obj_filename.c_str());
    string c;
    vertex.resize(0);
    edge.resize(0);
    color.resize(0);
    while (fin >> c)
    {
        if (c == "v")
        {
            float vx, vy, vz; fin >> vx >> vy >> vz;
            vertex.push_back(glm::vec3(vx, vy, vz));
            float cx, cy, cz; fin >> cx >> cy >> cz;
            color.push_back(glm::vec3(cx, cy, cz));
        }
        else if (c == "f")
        {
            if (fnum == 1)
            {
                int idx, idy, idz; fin >> idx >> idy >> idz;
                edge.push_back(glm::ivec3(idx - 1, idy - 1, idz - 1));
            }
            else
            {
                string st_idx, st_idy, st_idz; fin >> st_idx >> st_idy >> st_idz;
                int idx = atoi(st_idx.substr(0, st_idx.find('/')).c_str());
                int idy = atoi(st_idy.substr(0, st_idy.find('/')).c_str());
                int idz = atoi(st_idz.substr(0, st_idz.find('/')).c_str());
                edge.push_back(glm::ivec3(idx - 1, idy - 1, idz - 1));
            }
        }
    }
    fin.close();
}

static void ObjReader(vector<glm::vec3>& vertex, vector<glm::ivec3>& edge, 
                      const string obj_filename, const int f_num)
{
    ifstream fin(obj_filename.c_str());
	std::ios::sync_with_stdio(false);
    string c;
    vertex.resize(0);
    edge.resize(0);
    while (fin >> c)
    {
        if (c == "v")
        {
            float vx, vy, vz; fin >> vx >> vy >> vz;
            vertex.push_back(glm::vec3(vx, vy, vz));
        }
        else if (c == "f")
        {
            string st_idx, st_idy, st_idz; fin >> st_idx >> st_idy >> st_idz;
            int idx = atoi(st_idx.substr(0, st_idx.find('/')).c_str());
            int idy = atoi(st_idy.substr(0, st_idy.find('/')).c_str());
            int idz = atoi(st_idz.substr(0, st_idz.find('/')).c_str());
            edge.push_back(glm::ivec3(idx - 1, idy - 1, idz - 1));
        }
    }
    fin.close();
}

static void ObjWriter(vector<glm::vec3>& vertex, const vector<glm::ivec3>& edge,
                      const string obj_filename)
{
    ofstream fout(obj_filename.c_str());
    string c;
    
    vector<int> outliers(vertex.size(), 0);
    for (int i = 0; i < vertex.size(); i++)
    {
        if (abs(vertex[i].x) > 10 || abs(vertex[i].y) > 10 || abs(vertex[i].z) > 10)
        {
            outliers[i] = 1;
            vertex[i] = glm::vec3(0, 0, 0);
        }
        fout << "v " << vertex[i].x << ' ' << vertex[i].y << ' ' << vertex[i].z << endl;
        
    }
    for (int i = 0; i < edge.size(); i++)
    {
        if (!outliers[edge[i].x] && !outliers[edge[i].y] && !outliers[edge[i].z])
            fout << "f " << edge[i].x + 1 << ' ' << edge[i].y + 1 << ' ' << edge[i].z + 1 << endl;
    }

    fout.close();
}

static void ObjWriter(vector<glm::vec3>& vertex, const vector<glm::ivec3>& edge, vector<glm::vec3>& color,
                      const string obj_filename)
{
    ofstream fout(obj_filename.c_str());
    string c;

    vector<int> outliers(vertex.size(), 0);
    for (int i = 0; i < vertex.size(); i++)
    {
        if (abs(vertex[i].x) > 10 || abs(vertex[i].y) > 10 || abs(vertex[i].z) > 10)
        {
            outliers[i] = 1;
            vertex[i] = glm::vec3(0, 0, 0);
        }
        fout << "v " << vertex[i].x << ' ' << vertex[i].y << ' ' << vertex[i].z << ' ';
        fout << color[i].x << ' ' << color[i].y << ' ' << color[i].z << endl;
    }
    for (int i = 0; i < edge.size(); i++)
    {
        if (!outliers[edge[i].x] && !outliers[edge[i].y] && !outliers[edge[i].z])
            fout << "f " << edge[i].x + 1 << ' ' << edge[i].y + 1 << ' ' << edge[i].z + 1 << endl;
    }

    fout.close();
}

static void TransformVertex(vector<glm::vec3>& vertex,
                            const vector<glm::vec3>& vertex_origin,
                            const vector<vector<float> >& tr)
{
    for (int i = 0; i < vertex.size(); i++)
    {
        Eigen::Matrix3f r;
        r <<
            0, -tr[i][5], tr[i][4],
            tr[i][5], 0, -tr[i][3],
            -tr[i][4], tr[i][3], 0;
        Eigen::Vector3f v(vertex_origin[i].x, vertex_origin[i].y, vertex_origin[i].z);
        v = v + r*v + Eigen::Vector3f(tr[i][0], tr[i][1], tr[i][2]);
        vertex[i] = glm::vec3(v(0), v(1), v(2));
    }
}


static void TransformVertex(vector<glm::vec3>& vertex,
                            const glm::vec3 trans)
{
    for (int i = 0; i < vertex.size(); i++)
    {
        vertex[i] = vertex[i] + trans;
    }
    
}
