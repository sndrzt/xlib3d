all:
	g++ -o cilindro3d cilindro3d.c -lX11
	g++ -o cone3d cone3d.c -lX11
	g++ -o esfera3d esfera3d.c -lX11
	g++ -o torus3d torus3d.c -lX11
