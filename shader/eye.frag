
uniform sampler2D color_texture;
uniform sampler2D two_texture;
void main (void){
     //���������Ѿ��ڶ�����ɫ���д浽�����������ｫ��������������ͼƬ��ϾͿ�����
    vec4 color1 = texture(color_texture,gl_TexCoord[0].st);
    vec4 color2 = texture( two_texture ,gl_TexCoord[1].st);
	gl_FragColor = color1 + color2;
}
