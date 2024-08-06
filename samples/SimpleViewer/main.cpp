#define GLM_FORCE_UNRESTRICTED_GENTYPE
#include "Application.h"

int main()
{
	Application app("PhotonRenderer", 1600, 900);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}