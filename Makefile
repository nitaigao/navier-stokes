all:
	clang++ -I. NavierStokes/*.cpp -lglfw -lGLEW -lGL -o navier-stokes