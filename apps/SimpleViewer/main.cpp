#include "Application.h"

int main(int argc, char** argv)
{
	Application app("PhotonRenderer", 1600, 900);
	if (app.init())
	{
		app.loop();
	}
	app.shutdown();

	return 0;
}