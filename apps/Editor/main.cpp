#include "Application.h"

int main(int argc, char** argv)
{
	Application app("Editor", 1600, 900);
	if (app.init())
	{
		app.loop();
	}
	app.shutdown();

	return 0;
}