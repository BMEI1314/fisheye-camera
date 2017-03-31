#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glaux.h>
#include<iostream>  

using namespace std;
void display();
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glaux.lib")

//定义输出窗口的大小
#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH  1280
//z着色器句柄
GLuint G_vShader_simple;
GLuint G_fShader_simple;
GLuint G_shaderProgram;

static GLuint G_texNameArray[2];//纹理
GLdouble eqn[4] = {0.0,0.0,1.0,0.0};//半球切片
/****************************************************************************************/
#define LOAD_RGB24   0
#define LOAD_BGR24   0
#define LOAD_BGRA    0
#define LOAD_YUV420P 1          //读取的格式选择，

const int pixel_w = 1440, pixel_h =1080 ;   //视频的分辨率
/*******************************************************************************************/
//每个像素的bit
#if LOAD_BGRA
const int bpp=32;
#elif LOAD_RGB24|LOAD_BGR24
const int bpp=24;
#elif LOAD_YUV420P
const int bpp=12;
#endif
//YUV file
FILE *fp = NULL;
unsigned char buffer[pixel_w*pixel_h*bpp/8];
unsigned char buffer_convert[pixel_w*pixel_h*3];

inline unsigned char CONVERT_ADJUST(double tmp)
{
	return (unsigned char)((tmp >= 0 && tmp <= 255)?tmp:(tmp < 0 ? 0 : 255));
}
//YUV420P to RGB24
void CONVERT_YUV420PtoRGB24(unsigned char* yuv_src,unsigned char* rgb_dst,int nWidth,int nHeight)
{
	unsigned char *tmpbuf=(unsigned char *)malloc(nWidth*nHeight*3);
	unsigned char Y,U,V,R,G,B;
	unsigned char* y_planar,*u_planar,*v_planar;
	int rgb_width , u_width;
	rgb_width = nWidth * 3;
	u_width = (nWidth >> 1);
	int ypSize = nWidth * nHeight;
	int upSize = (ypSize>>2);
	int offSet = 0;

	y_planar = yuv_src;
	u_planar = yuv_src + ypSize;
	v_planar = u_planar + upSize;

	for(int i = 0; i < nHeight; i++)
	{
		for(int j = 0; j < nWidth; j ++)
		{
			// Get the Y value from the y planar
			Y = *(y_planar + nWidth * i + j);
			// Get the V value from the u planar
			offSet = (i>>1) * (u_width) + (j>>1);
			V = *(u_planar + offSet);
			// Get the U value from the v planar
			U = *(v_planar + offSet);

			// Cacular the R,G,B values
			// Method 1
			R = CONVERT_ADJUST((Y + (1.4075 * (V - 128))));
			G = CONVERT_ADJUST((Y - 0.3455 * (U - 128) - 0.7169 * (V - 128)));
			B = CONVERT_ADJUST((Y + (1.7790 * (U - 128))));
			/*
			// The following formulas are from MicroSoft' MSDN
			int C,D,E;
			// Method 2
			C = Y - 16;
			D = U - 128;
			E = V - 128;
			R = CONVERT_ADJUST(( 298 * C + 409 * E + 128) >> 8);
			G = CONVERT_ADJUST(( 298 * C - 100 * D - 208 * E + 128) >> 8);
			B = CONVERT_ADJUST(( 298 * C + 516 * D + 128) >> 8);
			R = ((R - 128) * .6 + 128 )>255?255:(R - 128) * .6 + 128; 
			G = ((G - 128) * .6 + 128 )>255?255:(G - 128) * .6 + 128; 
			B = ((B - 128) * .6 + 128 )>255?255:(B - 128) * .6 + 128; 
			*/
			offSet = rgb_width * i + j * 3;

			rgb_dst[offSet] = B;
			rgb_dst[offSet + 1] = G;
			rgb_dst[offSet + 2] = R;
		}
	}
	free(tmpbuf);
}
void timeFunc(int value){
    display();
    glutTimerFunc(1, timeFunc, 0);//帧率设置
}

/**************************************************************************************************/

//摄像机离物体的距离
float G_fDistance = 10.0f;
float G_RLDistance = -0.0f;
float G_UDDistance = -0.0f;
//物体的旋转角度 
float G_fAngle_horizon =180.0;
float G_fAngle_vertical =90.0f;
float G_scale =2.0f;

//光照参数
//float G_vLit0Position[4] = { 5.0f, 5.0f, 5.0f, 1.0f };
//float G_vLit0Ambient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
//float G_vLit0Diffuse[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
//float G_vLit0Specular[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
//
//float G_vMaterialAmbient[4] = { 0.0f, 0.8f, 0.0f, 1.0f };
//float G_vMaterialDiffuse[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
//float G_vMaterialSpecular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
//float G_iShininess = 50;

////////////////////////////////////////////////
void myinit(void);
void myReshape(GLsizei w, GLsizei h);
void display(void);
void setShaders(void);
void printShaderInfoLog(GLuint shaderObject);
void printProgramInfoLog(GLuint programObject);
char* textFileRead(const char *textFileName);


//响应键盘输入, 从而设定物体移近移远以及旋转的回调函数
void processSpecialKeys(int key, int x, int y);
void processNormalKeys(unsigned char key,int x,int y);


////////////////////////////////////////////////
//主函数
int main(int argc, char* argv[])
{ 
#if LOAD_BGRA
	fp=fopen("./Face2.avi","rb+");
#elif LOAD_RGB24
	fp=fopen("./test.mov","rb+");
#elif LOAD_BGR24
	fp=fopen("./face2.avi","rb+");
#elif LOAD_YUV420P
	fp=fopen("./output.yuv","rb+");
#endif
	if(fp==NULL){
		printf("Cannot open this file.\n");
		return -1;
	}
	//Mat img = imread("../test.jpg",1);
	glutInit(&argc, argv);

	//初始化OPENGL显示方式
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
	//glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA);

	//设定OPENGL窗口位置和大小
	glutInitWindowSize (WINDOW_WIDTH, WINDOW_HEIGHT); 
	glutInitWindowPosition (0, 0);
		
	//打开窗口
	glutCreateWindow("Simplest Video Play OpenGL");

	printf("ADWS上下左右移动， 方向键是选装，QEZX放大缩小\n");

	//调用初始化函数
    myinit();
	//loadTexImages();
	setShaders();
	//设定窗口大小变化的回调函数
	glutReshapeFunc(myReshape);

	//设定键盘控制的回调函数
	glutSpecialFunc(processSpecialKeys);
	glutKeyboardFunc(processNormalKeys);
	//开始OPENGL的循环
	glutDisplayFunc(display); 
	//glutDisplayFunc函数用于注册一个绘图函数， 这样操作系统在必要时刻就会对窗体进行重新绘制操作。类似于windows程序设计中处理WM_PAINT消息。具体来说呢，就是设置一个函数当需要进行画图时就调用这个函数如： glutDisplayFunc(display);

	//glutIdleFunc(display);//glutIdleFunc设置全局的回调函数，当没有窗口事件到达时，GLUT程序功能可以执行后台处理任务或连续动画。如果启用，这个idle function会被不断调用，直到有窗口事件发生。

	glutTimerFunc(100, timeFunc, 0); //帧率
	glutMainLoop();//glutMainLoop进入GLUT事件处理循环，让所有的与“事件”有关的函数调用无限循环。

	return 0;
}

////////////////////////////////////////////////
//用户初始化函数
void myinit(void)
{
    //your initialization code
	glEnable(GL_DEPTH_TEST);

	/*glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);*/
//	glEnable(GL_COLOR_MATERIAL);
//	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	GLenum err = glewInit();   
	if (GLEW_OK != err)   
	{   
		printf("glew initionlize error: %s\n", glewGetErrorString(err));
	}
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
		printf("Ready for GLSL\n");
	else {
		printf("Not totally ready \n");
		exit(1);
	}

	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else 
	{ 
		printf("OpenGL 2.0 not supported\n"); 
		exit(1);
	}

}

//窗口大小变化时的回调函数
void myReshape(GLsizei w, GLsizei h)//透视投影
{
	//设定视区
    glViewport(0, 0, w, h);

	//设定透视方式
    glMatrixMode(GL_PROJECTION);//GL_MODELVIEW 是模型矩阵GL_PROJECTION 是投影矩阵
    glLoadIdentity();
    gluPerspective(60.0, 1.0*(GLfloat)w/(GLfloat)h, 1.0, 300.0);
//	gluPerspective(60.0, 1.0, 1.0, 30.0);
 // glFrustum (-1.0, 1.0, -1.0, 1.0, 1.0, 30.0);
}

//每桢OpenGL都会调用这个函数，用户应该把显示代码放在这个函数中
void display(void)
{
	//设置清除屏幕的颜色，并清除屏幕和深度缓冲
	glGenTextures(1, G_texNameArray);
    
   
    if (fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp) != pixel_w*pixel_h*bpp/8){
        // Loop
        fseek(fp, 0, SEEK_SET);
        fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp);
    }


#if LOAD_BGRA
		glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pixel_w , pixel_h,GL_BGRA, GL_UNSIGNED_BYTE, buffer);
#elif LOAD_RGB24
	glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pixel_w ,pixel_h, GL_RGB, GL_UNSIGNED_BYTE, buffer);
#elif LOAD_BGR24
	glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pixel_w ,pixel_h, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
#elif LOAD_YUV420P
	CONVERT_YUV420PtoRGB24(buffer,buffer_convert,pixel_w,pixel_h);
		glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pixel_w , pixel_h, GL_RGB, GL_UNSIGNED_BYTE,buffer_convert);

#endif

    glClearColor(1.0f,1.0f,1.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(G_RLDistance, G_UDDistance, -G_fDistance);
	glRotatef(G_fAngle_horizon, 0.0f, 1.0f, 0.0f);
	glRotatef(G_fAngle_vertical, 1.0f, 0.0f, 0.0f);

	////////////////////////////////////////////////
	glClipPlane(GL_CLIP_PLANE0,eqn);
	/*void glClipPlane(GLenum plane, const GLdouble *equation); 
    定义一个裁剪平面。equation参数指向平面方程Ax + By + Cz + D = 0的4个系数。
	equation=（0，-1，0,0），前三个参数（0，-1,0）可以理解为法线向下，只有向下的，
	即Y<0的才能显示，最后一个参数0表示从z=0平面开始。这样就是裁剪掉上半平面。
	相应的equation=（0,1,0,0）表示裁剪掉下半平面，equation=（1,0,0,0）表示裁剪掉左半平面，
	equation=（-1,0,0,0）表示裁剪掉右半平面，
	equation=（0,0,-1,0）表示裁剪掉前半平面，equation=（0,0,1,0）表示裁剪掉后半平面*/
	glEnable(GL_CLIP_PLANE0);
	
	glUseProgram(G_shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);//OpenGl 函数。用于启用各种功能。具体功能由参数决定。与glDisable相对应。glDisable用以关闭各项功能。
	int texture_location = glGetUniformLocation(G_shaderProgram,"color_texture");
	glUniform1i(texture_location, 0);//对应纹理第一层
	glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	/* 对于贴了纹理的模型，可以使用 glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,XX)来指定纹理贴图和材质混合的方式，从而产生特定的绘制效果 */

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	int two_location = glGetUniformLocation(G_shaderProgram,"two_texture");
    
	glUniform1i(two_location, 1);//对应纹理第二层
	

	/***********************************************************************************/
	glutSolidSphere(G_scale,50,50);
	glClipPlane(GL_CLIP_PLANE0,eqn);
	/***********************************************************************************/
/*void glutSolidSphere(GLdouble radius , GLint slices , GLint stacks);
radius
球体的半径
slices
以Z轴上线段为直径分布的圆周线的条数（将Z轴看成地球的地轴，类似于经线）
stacks
围绕在Z轴周围的线的条数（类似于地球上纬线）
一般而言，后两个参数赋予较大的值，渲染花费的时间要长，效果更逼真。*/
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	glDisable(GL_CLIP_PLANE0);
                      
	glutSwapBuffers();
}

void processSpecialKeys(int key, int x, int y)//修改，限制角度
{ 
	switch(key) {//上下左右旋转
		case GLUT_KEY_LEFT:
			//if(G_fAngle_horizon>100.0)
			G_fAngle_horizon -= 10.0f;
			break;
		case GLUT_KEY_RIGHT:
			//if(G_fAngle_horizon<250.0)
			G_fAngle_horizon += 10.0f;
			break;
		case GLUT_KEY_UP:
			//if(G_fAngle_vertical>-160.0)
			G_fAngle_vertical -= 10.0f;
			break;
		case GLUT_KEY_DOWN:
			//if(G_fAngle_vertical<20.0)
			G_fAngle_vertical += 10.0f;
			break;
	}
	glutPostRedisplay();
}

void processNormalKeys(unsigned char key,int x,int y)
{
	  if(G_fDistance<3)
	{
   if(250>G_fAngle_horizon&&G_fAngle_horizon>100.0)
       G_fAngle_horizon=0;
   if(-20>G_fAngle_vertical&&G_fAngle_vertical>-160)
      G_fAngle_vertical=90;
    }
	switch(key) {
		case 'q':	//远近
			if(G_fDistance>-0.6)
			G_fDistance -= 0.3f;
			break;
		case 'Q':	
			if(G_fDistance>-0.6)
			G_fDistance -= 0.3f;
			break;
		case 'e':
		  if(G_fDistance>3)
	{
   if(250<G_fAngle_horizon||G_fAngle_horizon<100.0)
       G_fAngle_horizon=0;
   if(-20<G_fAngle_vertical||G_fAngle_vertical<-160)
      G_fAngle_vertical=90;
    }
			if(G_fDistance<10)
				G_fDistance += 0.3f;
			break;
		case 'E':	
			 if(G_fDistance>3)
	  {
         if(250>G_fAngle_horizon&&G_fAngle_horizon>100.0)   //收缩时调整角度
          G_fAngle_horizon=0;
         if(-20>G_fAngle_vertical&&G_fAngle_vertical>-160)
		   G_fAngle_vertical=-90;
	  }
			if(G_fDistance<10)
			G_fDistance += 0.3f;
			break;
		case 'A':	//左右
			G_RLDistance -= 0.3f;
			break;
		case 'a':		
			G_RLDistance -= 0.3f;
			break;
		case 'D':	
			G_RLDistance += 0.3f;
			break;
		case 'd':		
			G_RLDistance += 0.3f;
			break;
		case 'S':	//上下
			G_UDDistance -= 0.3f;
			break;
		case 's':		
			G_UDDistance -= 0.3f;
			break;
		case 'w':	
			G_UDDistance += 0.3f;
			break;
		case 'W':		
			G_UDDistance += 0.3f;
			break;
		case 'z':	//放大缩小
			G_scale += 0.1f;
			break;
		case 'Z':	
			G_scale += 0.1f;
			break;
		case 'x':	
			G_scale -= 0.1f;
			break;
		case 'X':	
			G_scale -= 0.1f;
			break;
		case 27:	//"esc"
			exit(0);
	}
	glutPostRedisplay();
}



/*在OpenGL整个程序的初始化阶段（一般是init()函数），做以下工作。
1、顶点着色程序的源代码和片段作色程序的源代码要分别保存到一个字符数组里面；
2、使用glCreateshader()分别创建一个顶点着色器对象和一个片段着色器对象；
3、使用glShaderSource()分别将顶点着色程序的源代码字符数组绑定到顶点着色器对象，将片段着色程序的源代码字符数组绑定到片段着色器对象；
4、使用glCompileShader()分别编译顶点着色器对象和片段着色器对象；
5、使用glCreaterProgram()创建一个（着色）程序对象；
6、使用glAttachShader()分别将顶点着色器对象和片段着色器对象附加到（着色）程序对象上；
7、使用glLinkProgram()对（着色）程序对象执行链接操作
8、使用glValidateProgram()对（着色）程序对象进行正确性验证
9、最后使用glUseProgram()将OpenGL渲染管道切换到着色器模式，并使用刚才做好的（着色）程序对象。
然后，才可以提交顶点。
*/
void setShaders(void)
{  
	char *vs,*fs; 
	vs = textFileRead("Shader/eye.vert");  
	fs = textFileRead("Shader/eye.frag");  
	const char *vv = vs;  
	const char *ff = fs;  

	G_vShader_simple = glCreateShader(GL_VERTEX_SHADER);  //建立 vertex_shader
	G_fShader_simple = glCreateShader(GL_FRAGMENT_SHADER); //建立 fragment_shader
    glShaderSource(G_vShader_simple, 1, &vv, NULL);  //分别将顶点着色程序的源代码字符数组绑定到顶点着色器对象，将片段着色程序的源代码字符数组绑定到片段着色器对象；
	glShaderSource(G_fShader_simple, 1, &ff, NULL);  

	free(vs);
	free(fs);
	/////////////////////////////////////////////////////////
	glCompileShader(G_vShader_simple); //分别编译顶点着色器对象和片段着色器对象； 
	glCompileShader(G_fShader_simple);  
	int checkResult;
	glGetShaderiv(G_vShader_simple, GL_COMPILE_STATUS, &checkResult);  
	if(GL_FALSE == checkResult)
	{
		printf("vertex shader compile error\n");
		printShaderInfoLog(G_vShader_simple);
	}
	glGetShaderiv(G_fShader_simple, GL_COMPILE_STATUS, &checkResult);  
	if(GL_FALSE == checkResult)
	{
		printf("fragment shader compile error\n");
		printShaderInfoLog(G_fShader_simple);
	}
	////////////////////////////////////////////////////////////
	G_shaderProgram = glCreateProgram(); 
	glAttachShader(G_shaderProgram, G_vShader_simple);  //分别将顶点着色器对象和片段着色器对象附加到（着色）程序对象上；
	glAttachShader(G_shaderProgram, G_fShader_simple); 
	glLinkProgram(G_shaderProgram);  
	glGetProgramiv(G_fShader_simple, GL_LINK_STATUS, &checkResult);  
	if(GL_FALSE == checkResult)
	{
		printf("shader link error\n");
		printProgramInfoLog(G_shaderProgram);
	}
}  

char* textFileRead(const char *textFileName)
{
	FILE *fp;
    if(NULL == (fp = fopen(textFileName, "r")))  
    {  
        printf("text file read error\n");  
		exit(1);  
    }  
  
    char ch;
	int fileLen = 0;
	//首先得到文件长度
	while(EOF != (ch=fgetc(fp)))  
    {  
        fileLen ++;  
    }

	char *fileStr = (char *)malloc((fileLen+1)*sizeof(char));
	//第二次读取文件
	rewind(fp);
	int i = 0;
    while(EOF != (ch=fgetc(fp)))  
    {  
        fileStr[i] = ch;
		i++;
    }  
	fileStr[fileLen] = '\0';	//注意这个一定要加。
  
    fclose(fp);
	return fileStr;
}

void printShaderInfoLog(GLuint shaderObject)
{
	GLint logLen = 0;
	GLint writtenLen = 0;
	GLchar* info_log;

	glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH , &logLen);       

	if (logLen > 1)
	{
		info_log = (GLchar*)malloc(logLen);
//		glGetInfoLogARB(shaderObject, logLen, &writtenLen, info_log);	//也许这是老版本的函数了。
		glGetShaderInfoLog(shaderObject, logLen, &writtenLen, info_log);  
//		printf("Information log: \n");
		printf("%s\n", info_log);
		free (info_log);
	}
}

void printProgramInfoLog(GLuint programObject)
{
	GLint logLen = 0;
	GLint writtenLen = 0;
	GLchar* info_log;

	glGetShaderiv(programObject, GL_INFO_LOG_LENGTH , &logLen);       

	if (logLen > 1)
	{
		info_log = (GLchar*)malloc(logLen);
//		glGetInfoLogARB(shaderObject, logLen, &writtenLen, info_log);
		glGetProgramInfoLog(programObject, logLen, &writtenLen, info_log);  
//		printf("Information log: \n");
		printf("%s\n", info_log);
		free (info_log);
	}
}