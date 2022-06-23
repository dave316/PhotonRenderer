#define GLM_FORCE_UNRESTRICTED_GENTYPE
#include "Application.h"

int main()
{
	Application app("PhotonRenderer", 1920, 1080);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}