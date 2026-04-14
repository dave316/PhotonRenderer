#include "Application.h"

int main(int argc, char** argv)
{
	Application app("GLTF Viewer", 2560, 1440);
	if (app.init())
	{
		app.loop();
	}
	app.shutdown();

	return 0;
}