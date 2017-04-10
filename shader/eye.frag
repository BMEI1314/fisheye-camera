
uniform sampler2D color_texture;
uniform sampler2D two_texture;
void main (void){
     //纹理坐标已经在顶点着色器中存到纹理矩阵里，这里将纹理坐标与纹理图片结合就可以了
    vec4 color1 = texture(color_texture,gl_TexCoord[0].st);
    vec4 color2 = texture( two_texture ,gl_TexCoord[1].st);
	gl_FragColor = color1 + color2;
}
