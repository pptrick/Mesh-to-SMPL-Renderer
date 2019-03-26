# Readme

目前的multiperson渲染可支持任意多个人体的渲染，并能够在一个合理的范围内随机生成观察位置和角度；渲染人数过多时可能会出现加载较慢的情况，同时渲染的人物可能会出现“穿膜”的情况，后期可以进一步优化速度并改进程序消除穿膜。

## multiperson （改进版）使用注意事项

使用时需要将multiperson.cpp和multiperson.h编译，multiperson.h的函数声明中仅有Cal_Joints（关键点计算函数）的定义来自main.cpp，其余函数均在multiperson.cpp中可找到定义。



具体使用时，只需要在主函数中加入如下一行代码即可输出结果：

```C++
MultiDisplay(text, window, "../res/vs_color.glsl", "../res/fs_color.glsl");
```

同时要在main.cpp开头包含头文件multiperson.h:

```C++
#include "multiperson.h"
```



非常需要注意的是，很多的文件名需要手动改写，包括上面这行代码的文件名，以及分布在multiperson.cpp中的：

```C++
string mesh_filename = "../data/mesh" + to_string(i+1) + ".obj";
string sample_filename = "../data/sampled_mesh" + to_string(i+1) + ".txt";
cv::imwrite("../output/color.png", render_results);
cv::imwrite("../output/depth.png", render_results);
string bbox_filename = "../output/BoundingBox" + to_string(i+1) + ".txt";
string front_filename = "../output/front" + to_string(j+1) + ".obj";
string coor_filename = "../output/coor" + to_string(j+1) + ".txt";
string Joints_filename = "../output/joints_coor" + to_string(i+1) + ".txt";
string dataJoints_filename = "../data/joints" + to_string(i+1) + ".txt";
string segment_filename = "../output/segment" + to_string(i + 1) + ".png";
string smpl_filename = "../data/smpl" + to_string(i + 1) + ".obj";
string sample_filename = "../data/sampled_mesh" + to_string(i + 1) + ".txt";

segment[i].Init("../res/vs_smpl.glsl", "../res/fs_smpl.glsl", smpl_filename, "../data/from_SMPL6890_to_IUV_norm.txt", sample_filename);//分割模型初始化

cv::imwrite("../output/complete_segment.png", segment_results);//多人分割模型写入
```

上述文件路径根据实际位置更改。注意上述string格式存储的路径中，to_string() 中的数字为人物模型编号，因此所有输入和输出文件名都应该命名为：../../../xxxnum.格式，例如：

```C++
"../output/segment2.png" //"2"表示第二个人物
```



原来的Cal_Joints函数（main.cpp）需要修改成如下形式：

即在倒数第三行处Joints[i].y前面加一个负号、第一行形参将const char* 改成 string、同时再将所有Model改成model:

```C++
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
```



由于需要在程序中更改文件名，我将所有const char* 格式存储的文件名基本改成了string格式，需要更改的部分包括（multiperson.cpp和multiperson.h中的已经修改好了，下面是其他文件中需要更改的部分，不确定的话可以参照文件包中的其他文件，那些都已经修改好）：

```C++
//main.cpp:
void Cal_Joints(string in_filename, string out_filename,glm::mat4 model, glm::mat4 view, glm::mat4 projection)
    
//cal_smpl.cpp & cal_smpl.h:
void Init(const string vs_name, const string fs_name, string smpl_filename, const char* seg_filename, string sample_filename);
void display(GLFWwindow* window, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const string segment_filename);
void load_segment_data(string filename);
void load_smpl_data(string filename);
void load_sample_data(string filename);

```



若需要修改人物个数，首先应该修改文件格式（包含后缀1、2、3...），然后需要修改NUM值：

```C++
#define NUM 3 //人体个数
```

下面函数是一个位置角度随机生成器：

```C++
void GenPosition(float& angle1, float& angle2, glm::vec3& trans, int seed)
{
	srand(seed + 100 * time(0));
	angle1 = rand() % 30 + 75;
	angle2 = rand() % 180 - 90;

	trans.x = (rand() % 12) / 10 - 0.6;
	trans.y = (rand() % 12) / 10 - 0.6;
	trans.z = (rand() % 16) / 10 + 0.5;
}
```

其中参数我均调试过，可以根据应用做一些微调（但是一定要注意，angle1范围要在0~90度，angle2范围须在-90~90度，同时trans不可过大，否则会移除屏幕）


