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

//����������ڵĴ�С
#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH  1280
//z��ɫ�����
GLuint G_vShader_simple;
GLuint G_fShader_simple;
GLuint G_shaderProgram;

static GLuint G_texNameArray[2];//����
GLdouble eqn[4] = {0.0,0.0,1.0,0.0};//������Ƭ
/****************************************************************************************/
#define LOAD_RGB24   0
#define LOAD_BGR24   0
#define LOAD_BGRA    0
#define LOAD_YUV420P 1          //��ȡ�ĸ�ʽѡ��

const int pixel_w = 1440, pixel_h =1080 ;   //��Ƶ�ķֱ���
/*******************************************************************************************/
//ÿ�����ص�bit
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
    glutTimerFunc(1, timeFunc, 0);//֡������
}

/**************************************************************************************************/

//�����������ľ���
float G_fDistance = 10.0f;
float G_RLDistance = -0.0f;
float G_UDDistance = -0.0f;
//�������ת�Ƕ� 
float G_fAngle_horizon =180.0;
float G_fAngle_vertical =90.0f;
float G_scale =2.0f;

//���ղ���
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


//��Ӧ��������, �Ӷ��趨�����ƽ���Զ�Լ���ת�Ļص�����
void processSpecialKeys(int key, int x, int y);
void processNormalKeys(unsigned char key,int x,int y);


////////////////////////////////////////////////
//������
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

	//��ʼ��OPENGL��ʾ��ʽ
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
	//glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA);

	//�趨OPENGL����λ�úʹ�С
	glutInitWindowSize (WINDOW_WIDTH, WINDOW_HEIGHT); 
	glutInitWindowPosition (0, 0);
		
	//�򿪴���
	glutCreateWindow("Simplest Video Play OpenGL");

	printf("ADWS���������ƶ��� �������ѡװ��QEZX�Ŵ���С\n");

	//���ó�ʼ������
    myinit();
	//loadTexImages();
	setShaders();
	//�趨���ڴ�С�仯�Ļص�����
	glutReshapeFunc(myReshape);

	//�趨���̿��ƵĻص�����
	glutSpecialFunc(processSpecialKeys);
	glutKeyboardFunc(processNormalKeys);
	//��ʼOPENGL��ѭ��
	glutDisplayFunc(display); 
	//glutDisplayFunc��������ע��һ����ͼ������ ��������ϵͳ�ڱ�Ҫʱ�̾ͻ�Դ���������»��Ʋ�����������windows��������д���WM_PAINT��Ϣ��������˵�أ���������һ����������Ҫ���л�ͼʱ�͵�����������磺 glutDisplayFunc(display);

	//glutIdleFunc(display);//glutIdleFunc����ȫ�ֵĻص���������û�д����¼�����ʱ��GLUT�����ܿ���ִ�к�̨�������������������������ã����idle function�ᱻ���ϵ��ã�ֱ���д����¼�������

	glutTimerFunc(100, timeFunc, 0); //֡��
	glutMainLoop();//glutMainLoop����GLUT�¼�����ѭ���������е��롰�¼����йصĺ�����������ѭ����

	return 0;
}

////////////////////////////////////////////////
//�û���ʼ������
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

//���ڴ�С�仯ʱ�Ļص�����
void myReshape(GLsizei w, GLsizei h)//͸��ͶӰ
{
	//�趨����
    glViewport(0, 0, w, h);

	//�趨͸�ӷ�ʽ
    glMatrixMode(GL_PROJECTION);//GL_MODELVIEW ��ģ�;���GL_PROJECTION ��ͶӰ����
    glLoadIdentity();
    gluPerspective(60.0, 1.0*(GLfloat)w/(GLfloat)h, 1.0, 300.0);
//	gluPerspective(60.0, 1.0, 1.0, 30.0);
 // glFrustum (-1.0, 1.0, -1.0, 1.0, 1.0, 30.0);
}

//ÿ��OpenGL�����������������û�Ӧ�ð���ʾ����������������
void display(void)
{
	//���������Ļ����ɫ���������Ļ����Ȼ���
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
    ����һ���ü�ƽ�档equation����ָ��ƽ�淽��Ax + By + Cz + D = 0��4��ϵ����
	equation=��0��-1��0,0����ǰ����������0��-1,0���������Ϊ�������£�ֻ�����µģ�
	��Y<0�Ĳ�����ʾ�����һ������0��ʾ��z=0ƽ�濪ʼ���������ǲü����ϰ�ƽ�档
	��Ӧ��equation=��0,1,0,0����ʾ�ü����°�ƽ�棬equation=��1,0,0,0����ʾ�ü������ƽ�棬
	equation=��-1,0,0,0����ʾ�ü����Ұ�ƽ�棬
	equation=��0,0,-1,0����ʾ�ü���ǰ��ƽ�棬equation=��0,0,1,0����ʾ�ü������ƽ��*/
	glEnable(GL_CLIP_PLANE0);
	
	glUseProgram(G_shaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);//OpenGl �������������ø��ֹ��ܡ����幦���ɲ�����������glDisable���Ӧ��glDisable���Թرո���ܡ�
	int texture_location = glGetUniformLocation(G_shaderProgram,"color_texture");
	glUniform1i(texture_location, 0);//��Ӧ�����һ��
	glBindTexture(GL_TEXTURE_2D, G_texNameArray[0]);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	/* �������������ģ�ͣ�����ʹ�� glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,XX)��ָ��������ͼ�Ͳ��ʻ�ϵķ�ʽ���Ӷ������ض��Ļ���Ч�� */

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	int two_location = glGetUniformLocation(G_shaderProgram,"two_texture");
    
	glUniform1i(two_location, 1);//��Ӧ����ڶ���
	

	/***********************************************************************************/
	glutSolidSphere(G_scale,50,50);
	glClipPlane(GL_CLIP_PLANE0,eqn);
	/***********************************************************************************/
/*void glutSolidSphere(GLdouble radius , GLint slices , GLint stacks);
radius
����İ뾶
slices
��Z�����߶�Ϊֱ���ֲ���Բ���ߵ���������Z�ῴ�ɵ���ĵ��ᣬ�����ھ��ߣ�
stacks
Χ����Z����Χ���ߵ������������ڵ�����γ�ߣ�
һ����ԣ���������������ϴ��ֵ����Ⱦ���ѵ�ʱ��Ҫ����Ч�������档*/
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

void processSpecialKeys(int key, int x, int y)//�޸ģ����ƽǶ�
{ 
	switch(key) {//����������ת
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
		case 'q':	//Զ��
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
         if(250>G_fAngle_horizon&&G_fAngle_horizon>100.0)   //����ʱ�����Ƕ�
          G_fAngle_horizon=0;
         if(-20>G_fAngle_vertical&&G_fAngle_vertical>-160)
		   G_fAngle_vertical=-90;
	  }
			if(G_fDistance<10)
			G_fDistance += 0.3f;
			break;
		case 'A':	//����
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
		case 'S':	//����
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
		case 'z':	//�Ŵ���С
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



/*��OpenGL��������ĳ�ʼ���׶Σ�һ����init()�������������¹�����
1��������ɫ�����Դ�����Ƭ����ɫ�����Դ����Ҫ�ֱ𱣴浽һ���ַ��������棻
2��ʹ��glCreateshader()�ֱ𴴽�һ��������ɫ�������һ��Ƭ����ɫ������
3��ʹ��glShaderSource()�ֱ𽫶�����ɫ�����Դ�����ַ�����󶨵�������ɫ�����󣬽�Ƭ����ɫ�����Դ�����ַ�����󶨵�Ƭ����ɫ������
4��ʹ��glCompileShader()�ֱ���붥����ɫ�������Ƭ����ɫ������
5��ʹ��glCreaterProgram()����һ������ɫ���������
6��ʹ��glAttachShader()�ֱ𽫶�����ɫ�������Ƭ����ɫ�����󸽼ӵ�����ɫ����������ϣ�
7��ʹ��glLinkProgram()�ԣ���ɫ���������ִ�����Ӳ���
8��ʹ��glValidateProgram()�ԣ���ɫ��������������ȷ����֤
9�����ʹ��glUseProgram()��OpenGL��Ⱦ�ܵ��л�����ɫ��ģʽ����ʹ�øղ����õģ���ɫ���������
Ȼ�󣬲ſ����ύ���㡣
*/
void setShaders(void)
{  
	char *vs,*fs; 
	vs = textFileRead("Shader/eye.vert");  
	fs = textFileRead("Shader/eye.frag");  
	const char *vv = vs;  
	const char *ff = fs;  

	G_vShader_simple = glCreateShader(GL_VERTEX_SHADER);  //���� vertex_shader
	G_fShader_simple = glCreateShader(GL_FRAGMENT_SHADER); //���� fragment_shader
    glShaderSource(G_vShader_simple, 1, &vv, NULL);  //�ֱ𽫶�����ɫ�����Դ�����ַ�����󶨵�������ɫ�����󣬽�Ƭ����ɫ�����Դ�����ַ�����󶨵�Ƭ����ɫ������
	glShaderSource(G_fShader_simple, 1, &ff, NULL);  

	free(vs);
	free(fs);
	/////////////////////////////////////////////////////////
	glCompileShader(G_vShader_simple); //�ֱ���붥����ɫ�������Ƭ����ɫ������ 
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
	glAttachShader(G_shaderProgram, G_vShader_simple);  //�ֱ𽫶�����ɫ�������Ƭ����ɫ�����󸽼ӵ�����ɫ����������ϣ�
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
	//���ȵõ��ļ�����
	while(EOF != (ch=fgetc(fp)))  
    {  
        fileLen ++;  
    }

	char *fileStr = (char *)malloc((fileLen+1)*sizeof(char));
	//�ڶ��ζ�ȡ�ļ�
	rewind(fp);
	int i = 0;
    while(EOF != (ch=fgetc(fp)))  
    {  
        fileStr[i] = ch;
		i++;
    }  
	fileStr[fileLen] = '\0';	//ע�����һ��Ҫ�ӡ�
  
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
//		glGetInfoLogARB(shaderObject, logLen, &writtenLen, info_log);	//Ҳ�������ϰ汾�ĺ����ˡ�
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