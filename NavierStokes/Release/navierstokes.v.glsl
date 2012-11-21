uniform mat4 Projection;
uniform mat4 View;

void main() {
	gl_Position = Projection * View * gl_Vertex;	
	gl_FrontColor = gl_Color;
}