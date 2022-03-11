#include "Editor.h"

int main()
{
	Editor editor("Scene Editor", 1920, 1080);
	if (editor.init())
	{
		editor.loop();
	}
	return 0;
}
