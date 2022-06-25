#include "Application.h"

int main()
{
	Application app("GLTF Viewer", 1600, 900);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}
