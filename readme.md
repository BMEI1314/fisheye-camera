### 我隐去shader(glsl),如有想要研究的，请e-mail me！！！





  ###                                                  鱼眼矫正



​                                                                         (vs2012+opengl)

参考：

[雷霄骅OPENGL博客]: http://blog.csdn.net/leixiaohua1020/article/details/40379845

在这里用到了一种新的语言：OpenGL Shader Language,简称GLSL。它是一种类似于C语言的专门为GPU设计的语言，它可以放在GPU里面被并行运行。
OpenGL的着色器有.frag和.vert两个文件。这两个文件在被编译和链接后就可以产生可执行程序与GPU交互。

.vert 是Vertex Shader（顶点着色器），用于顶点计算，可以理解控制顶点的位置，在这个文件中我们通常会传入当前顶点的位置，和纹理的坐标。

.frag 是Fragment Shader（片元着色器），在这里面我可以对于每一个像素点进行重新计算。

#### 1.说明

Input:处理YUV420P流媒体，

output：球形渲染







#### 2.缺点

A:缺少相机标定，默认180°（市面上的鱼眼摄像头一般也是180，r=2f/sin(theta/2),f是焦距，r是像素图片中心的距离）。

B:对于输入的每帧图片缺少边缘检测,边缘有黑框会影响效果。（一般不会有黑框对于h.264解码后的视频）

C:gluBuild2DMipmaps只能处理 宽高为偶数的图像（前面加一个判断条件）

#### 3.跑了一下摄像头的视频

![](./img/1.png)



![](./img/2.png)

![](./img/3.png)
