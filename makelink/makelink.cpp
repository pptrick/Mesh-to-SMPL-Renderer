// makelink.cpp: 定义控制台应用程序的入口点。
// 从smpl model到mesh的对应点实现 

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>

using namespace std;

//smpl上点的类
class point
{
public:
	double x;
	double y;
	double z;
	bool handjudge;
	int to_num;
	double to_x;
	double to_y;
	double to_z;


	point() {};
	void init_point(double x0, double y0, double z0)
	{
		x = x0; y = y0; z = z0; to_num = 0; to_x = 0; to_y = 0;
		handjudge = 1;
	};
};

//mesh上点的类
class mesh_coor
{
public:
	double mesh_x;
	double mesh_y;
	double mesh_z;
	int mesh_num;  //mesh_num从0开始存储，按照obj读入顺序索引
	mesh_coor() {};
};

//正在处理的模型（含smpl和mesh）的类
class model
{
public:
	vector<point> data;  //smpl上的每一点，目标是寻找这些点的对应
	vector<mesh_coor> ref;  //mesh上的每一点，中间某些点将成为data的对应
	void loadsmpl(const char *filename);  
	void loadmesh(const char *filename);  
	void loadhandinf(const char *filename);
	void link();
};

//3D_tree节点的结构体
struct node
{
	double node_x;
	double node_y;
	double node_z;
	int node_num;
	int direction;  //0表示x方向，1表示y方向，2表示z方向
	node *l_child;
	node *r_child;
};

node *root;  //用于始终指向根节点

//比较函数，用于排序找中位点以及分割左右子树
bool comparison_x(mesh_coor a, mesh_coor b)
{
	return a.mesh_x < b.mesh_x;
}

bool comparison_y(mesh_coor a, mesh_coor b)
{
	return a.mesh_y < b.mesh_y;
}

bool comparison_z(mesh_coor a, mesh_coor b)
{
	return a.mesh_z < b.mesh_z;
}

//距离函数，计算两点之间的距离（平方）
double distance(point a,node *b)
{
	return (a.x - b->node_x)*(a.x - b->node_x) + (a.y - b->node_y)*(a.y - b->node_y) + (a.z - b->node_z)*(a.z - b->node_z);
}

//存储smpl的点云数据
void model::loadsmpl(const char *filename)
{
	ifstream fin(filename);
	point temp;
	char category='v';
	double coor[3];
	while (fin >> category&&(int)category==118)
	{
		for (int i = 0; i < 3; i++)
		{
			fin >> coor[i];
		}
		temp.init_point(coor[0], coor[1], coor[2]);
		data.push_back(temp);
	}
	fin.close();
}

//存储smpl手的判断数据
void model::loadhandinf(const char *filename)
{
	ifstream fin(filename);
	for (int i = 0; i < data.size(); i++)
	{
		fin >> data[i].handjudge;
	}
	fin.close();
}

//存储mesh的点云数据
void model::loadmesh(const char *filename)
{
	ifstream fin(filename);
	char category='v';
	double coor[3];
	string t;
	mesh_coor temp;
	int num = 0;
	int location;
	while (fin >> category && (int)category == 118)
	{
		fin >> temp.mesh_x >> temp.mesh_y >> temp.mesh_z;
		getline(fin, t);
		getline(fin, t);
		temp.mesh_num = num;
		ref.push_back(temp);
		num++;
	}
	sort(ref.begin(), ref.end(), comparison_x);
	fin.close();
}
//以上两个存储函数针对obj格式文件，其他文件需修改读取方式

//以下是kd tree搜索算法

//树的递归构建函数
bool build_tree(int d, vector<mesh_coor> ref, node *&p)
{
	p = new node;
	if (ref.empty()) { p = NULL; return 0; }
	int central;
	vector<mesh_coor> left;
	vector<mesh_coor> right;
	if (d % 3 == 1)
	{
		sort(ref.begin(), ref.end(), comparison_y);
		central = ref.size() / 2;
		p->node_x = ref[central].mesh_x; p->node_y = ref[central].mesh_y; p->node_z = ref[central].mesh_z;
		p->direction = 1; p->node_num = ref[central].mesh_num;
		for (int i = 0; i < ref.size(); i++)
		{
			if (ref[i].mesh_y < ref[central].mesh_y)left.push_back(ref[i]);
			else if (ref[i].mesh_y > ref[central].mesh_y)right.push_back(ref[i]);
		}
	}
	else if (d % 3 == 2)
	{
		sort(ref.begin(), ref.end(), comparison_z);
		central = ref.size() / 2;
		p->node_x = ref[central].mesh_x; p->node_y = ref[central].mesh_y; p->node_z = ref[central].mesh_z;
		p->direction = 2; p->node_num = ref[central].mesh_num;
		for (int i = 0; i < ref.size(); i++)
		{
			if (ref[i].mesh_z < ref[central].mesh_z)left.push_back(ref[i]);
			else if (ref[i].mesh_z > ref[central].mesh_z)right.push_back(ref[i]);
		}
	}
	else
	{
		sort(ref.begin(), ref.end(), comparison_x);
		central = ref.size() / 2;
		p->node_x = ref[central].mesh_x; p->node_y = ref[central].mesh_y; p->node_z = ref[central].mesh_z;
		p->direction = 0; p->node_num = ref[central].mesh_num;
		for (int i = 0; i < ref.size(); i++)
		{
			if (ref[i].mesh_x < ref[central].mesh_x)left.push_back(ref[i]);
			else if (ref[i].mesh_x > ref[central].mesh_x)right.push_back(ref[i]);
		}
	}

	build_tree(d+1, left, p->l_child);
	build_tree(d+1, right, p->r_child);
}

//树的初始化函数（含build tree）
void init_tree(vector<mesh_coor> ref)
{
	int depth = 0;
	node *p = new node;
	root = p;
	int central;

	sort(ref.begin(), ref.end(), comparison_x);
	central = ref.size() / 2;
	p->node_x = ref[central].mesh_x; p->node_y = ref[central].mesh_y; p->node_z = ref[central].mesh_z;
	p->direction = 0; p->node_num = ref[central].mesh_num;
	
	vector<mesh_coor> left;
	vector<mesh_coor> right;
	for (int i = 0; i < ref.size(); i++)
    {
		if (ref[i].mesh_x < ref[central].mesh_x)left.push_back(ref[i]);
		else if (ref[i].mesh_x > ref[central].mesh_x)right.push_back(ref[i]);
	}

	depth++;
	build_tree(depth, left, p->l_child);
	build_tree(depth, right, p->r_child);
}

//几何最近邻搜索函数
void recurSearch(node *&nearest, node *tracker, point smpl)
{
	if (tracker == NULL) return;
	if (distance(smpl, tracker) < distance(smpl, nearest))
	{
		nearest = tracker;
	}
	if (tracker->direction == 0)
	{
		if (smpl.x < tracker->node_x)
		{
			recurSearch(nearest, tracker->l_child, smpl);
			if ((smpl.x - tracker->node_x)*(smpl.x - tracker->node_x) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->r_child, smpl);
			}
		}
		else
		{
			recurSearch(nearest, tracker->r_child, smpl);
			if ((smpl.x - tracker->node_x)*(smpl.x - tracker->node_x) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->l_child, smpl);
			}
		}
	}
	else if (tracker->direction == 1)
	{
		if (smpl.y < tracker->node_y)
		{
			recurSearch(nearest, tracker->l_child, smpl);
			if ((smpl.y - tracker->node_y)*(smpl.y - tracker->node_y) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->r_child, smpl);
			}
		}
		else
		{
			recurSearch(nearest, tracker->r_child, smpl);
			if ((smpl.y - tracker->node_y)*(smpl.y - tracker->node_y) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->l_child, smpl);
			}
		}
	}
	else
	{
		if (smpl.z < tracker->node_z)
		{
			recurSearch(nearest, tracker->l_child, smpl);
			if ((smpl.z - tracker->node_z)*(smpl.z - tracker->node_z) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->r_child, smpl);
			}
		}
		else
		{
			recurSearch(nearest, tracker->r_child, smpl);
			if ((smpl.z - tracker->node_z)*(smpl.y - tracker->node_z) < distance(smpl, nearest))
			{
				recurSearch(nearest, tracker->l_child, smpl);
			}
		}
	}

}

//将最近邻与smpl model链接
void model::link()
{
	for (int i = 0; i < data.size(); i++)
    {
		if (data[i].handjudge == 0)continue;
		node *nearest = root;
		recurSearch(nearest, root, data[i]);
		data[i].to_x = nearest->node_x;
		data[i].to_y = nearest->node_y;
		data[i].to_z = nearest->node_z;
		data[i].to_num = nearest->node_num;
	}
}

//kd_tree搜索结束

//输出测试函数（们）
void out_test(const char *filename,model a)
{
	ofstream fout(filename);
	for (int i = 0; i < a.data.size(); i++)
	{
		if (a.data[i].handjudge == 1)
		{
			fout << 'v' << " " << a.data[i].to_x << " " << a.data[i].to_y << " " << a.data[i].to_z << endl;
		}
	}
}

void out_test_node(const char *filename,node *p)
{
	ofstream fout(filename,ios::app);
	if (p == NULL)return;
	fout << 'v' << " " << p->node_x << " " << p->node_y << " " << p->node_z << endl;
	out_test_node(filename, p->l_child);
	out_test_node(filename, p->r_child);
}

void out_test_mesh(const char *filename, model a)
{
	ofstream fout(filename);
	for (int i = 0; i < a.ref.size(); i++)
	{
		fout << 'v' << " " << a.ref[i].mesh_x << " " << a.ref[i].mesh_y << " " << a.ref[i].mesh_z << endl;
	}
}

void out_test_smpl(const char *filename, model a)
{
	ofstream fout(filename);
	for (int i = 0; i < a.data.size(); i++)
	{
		if (a.data[i].handjudge == 0)continue;
		fout << 'v' << " " << a.data[i].x << " " << a.data[i].y << " " << a.data[i].z << endl;
	}
}

int main()
{
	clock_t t = clock();
	model pcy;
	pcy.loadsmpl("../data/frame_3491_live_smpl.obj");
	pcy.loadmesh("../data/frame_3491_live_mesh.obj");
	pcy.loadhandinf("../data/eliminationFlagFile.txt");
	//pcy.loadmesh("../../test.obj");
	init_tree(pcy.ref);
	pcy.link();
	t = clock() - t;
	cout<<"time using : "<< (float(t) / CLOCKS_PER_SEC) << "seconds\n";

	//out_test("model_test.obj",pcy);
	//out_test_node("model_test.txt", root);
	//out_test_mesh("model_test.txt", pcy);
	out_test_smpl("model_test.obj", pcy);

    return 0;
}


