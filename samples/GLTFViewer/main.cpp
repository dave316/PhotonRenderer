#include "Application.h"

int main()
{
	Application app("GLTF Viewer", 1920, 1080);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}
