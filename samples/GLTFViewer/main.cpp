#include "Application.h"

int main()
{
	Application app("GLTF Viewer", 1280, 720);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}
