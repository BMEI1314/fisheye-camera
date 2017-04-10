
void main(){
    vec3 npos=normalize(gl_Vertex.xyz);//点的朝向矢量
    vec2 px=npos.xz;
    float sinthita=length(px);
    px=normalize(px);
    float thita=asin(sinthita);
	float py=2.0*sin(thita*0.5);	
	//gl_TexCoord[0].st=px*py*(1.0/pow(2.0,0.5))*0.5+0.5; 

	if(gl_Vertex.y<0)
	   gl_TexCoord[0].st=px*py*(1.0/pow(2.0,0.5))*0.5+0.5; 	
	else
	   gl_TexCoord[1].st=px*py*(1.0/pow(2.0,0.5))*0.5+0.5; 

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}