uniform mat4 Projection;
uniform mat4 View;

attribute vec4 position;
attribute vec4 color;

void main() {
	gl_Position = Projection * View * position;	
	gl_FrontColor = color;
}