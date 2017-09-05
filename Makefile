all: pcompile

pcompile: billiards.cpp
	g++ billiards.cpp vector3.cpp mySphere.cpp -o billiards -lgluix -lglui -lglut32 -lglu32 -lopengl32 -lgdi32 -lalut -lopenal32
