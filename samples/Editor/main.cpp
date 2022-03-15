#include "Editor.h"

int main()
{
	Editor editor("Scene Editor", 1600, 900);
	if (editor.init())
	{
		editor.loop();
	}
	return 0;
}
