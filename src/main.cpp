#include "Application.h"

int main()
{
	std::cout << sizeof(glm::u16vec4) << std::endl;

	Application app("PhotonRenderer", 1920, 1080);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}