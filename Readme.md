# Mesh_SMPL Renderer

此渲染器可用于多角度含人体的RGB、Depth以及身体部位分割图的渲染工作；主要功能分为以下几类：

1、在同坐标框架下找到double fusion所采集到inner layer(SMPL)和outer layer(mesh)之间点的对应关系

2、给定mesh和背景图片，生成含有光照的2DRGB合成图片，其中可包含多个人体以及多个角度

3、将“1”中生成的对应文件加入渲染器，可以生成mesh上点到2DRGB图片上像素点的对应关系

4、加入SMPL和分割条件，可以生成2D的mesh分割图



## Mesh_to_SMPL makelink

此部分的主要功能是实现在同坐标框架下找到double fusion所采集到inner layer(SMPL)和outer layer(mesh)之间点的对应关系。输入为一个mesh三维模型（obj格式）以及其对应的SMPL model，输出为点对应文件，文件包含6890行，按顺序对应SMPL model上6890个点，“v”后接三个量是指该SMPL点对应的mesh的(x,y,z)坐标：

```C++
...
v 1.05326 0.927912 0.602565
v 1.0425 0.942224 0.598045
...
```

下面为各对应点的三维效果图：

<img src="https://github.com/pptrick/Mesh-to-SMPL-Renderer/blob/master/readme_pic/makelink.png" width="300"/>

找对应点的方法是利用同框架mesh和SMPL模型比较相近的特点，寻找每个SMPL上的点在mesh上的（几何意义上）最近邻。

具体实现采用了kdtree数据结构，将mesh的点按照空间分布存储到kdtree中。树的递归构建函数build_tree()将点保序依次按照x方向、y方向、z方向的顺序存入kdtree的分支。

对于每一个SMPL上的点，调用最近邻搜索recurSearch()函数。该函数可通过剪枝去除大量不需要直接比较的点从而达到快速寻找最近邻的目的。剪枝思想是若某区域到目标点的最近点（一般是到面的垂直距离）距离大于当前最近邻到目标点的距离，则剪枝。

<u>**limitation**</u>：由于SMPL和mesh手形不同，手的部分对应点可能会寻找到身体的其他部位



## RGB Rendering

RGB Rendering可将一个或多个mesh模型渲染到一张RGB背景图像上，并为其添加光照效果：

<img src="https://github.com/pptrick/Mesh-to-SMPL-Renderer/blob/master/readme_pic/RGB_Rendering.png" width="300"/>

具体实现方法是将模型通过矩阵变换投影到二维平面上。输入为mesh模型和背景图片，输出为渲染出来的2DRGB合成图。此处采用了openGL以及openCV图像处理框架。

在openGL图形渲染管线中将顶点信息（位置、颜色以及法向）存入顶点缓冲对象（VBO），将顶点链接信息（面片）存入索引缓冲对象（EBO）。将上述对象传入渲染管线，依次通过顶点着色器、几何着色器以及片段着色器。并将整个过程绑定在顶点数组对象（VAO）上。

对于可编程管线的openGL，顶点着色器和片段着色器程序均可编辑，具体程序存储在glsl文件中。

实际渲染过程是将mesh的顶点信息传入，修改变换和投影矩阵，将渲染结果在窗口中输出，并使用openCV记录图片。

此处的光照采用的是Phong局域光照模型，并未考虑全局光照。

深度图渲染与上述渲染过程相似，只需要将颜色信息改换成z方向坐标信息即可：

<img src="https://github.com/pptrick/Mesh-to-SMPL-Renderer/blob/master/readme_pic/depth_Rendering.png" width="300"/>

<u>**limitation**</u>: 对于很多人体的情况，仍然会出现穿模问题；同时当人体非常靠近“镜头”时，会出现畸变现象。



## Mesh_RGB Match

此部分将mesh和渲染出来的图像上的像素点建立了对应关系。

输入为Mesh_to_SMPL makelink中输出的对应文件，输出为若干行，每行三个整数：

```C++
...
54 175 187
55 177 190
...
```

所谓mesh上的点，其实是mesh上有“SMPL模型对应”的点，因此这些点必然存在一个SMPL点的对应。第一个整数为对应SMPL点的序号，后两个为渲染图像上该点的像素坐标。此时RGB到mesh再到SMPL的完整对应关系已经完成了。

此部分只会输出“可见”的mesh上的点，被遮挡或背面不可见的点不会被输出（因为没有像素可对应）。判断“是否可见”采用了深度判定的方法，比较某个点的z值和投影到depth图像上的实际深度，若相同则为可见。生成文件中的front.obj文件中，绿点为可见点，红点为不可见点。



## Body Segmentation

Body Segmentation可输出mesh根据不同身体部位的分割渲染图（SMPL也可）：

<img src="https://github.com/pptrick/Mesh-to-SMPL-Renderer/blob/master/readme_pic/complete_segment.png" width="300"/>

此处只需利用SMPL model的身体部位信息，将mesh上有SMPL对应的各个顶点“身体部位信息”标定即可。其中身体部位信息可转换为颜色信息输出。

<u>**limitation**</u>: 手部与肢体有粘连，不同部位的分割存在边界模糊（被插值）的情况



## For Users

具体使用时，需要单独运行makelink和render。

详见每个文件的readme
