#include "Application.h"

int main()
{
	std::cout << "sizeof(float): " << sizeof(float) << std::endl;
	std::cout << "sizeof(glm::vec2): " << sizeof(glm::vec2) << std::endl;
	std::cout << "sizeof(glm::vec3): " << sizeof(glm::vec3) << std::endl;
	std::cout << "sizeof(glm::vec4): " << sizeof(glm::vec4) << std::endl;
	std::cout << "sizeof(glm::quat): " << sizeof(glm::quat) << std::endl;

	Application app("PhotonRenderer", 1920, 1080);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}