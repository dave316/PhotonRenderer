#include "Editor.h"

int main()
{
	Editor editor("Scene Editor", 2560, 1440);
	if (editor.init())
	{
		editor.loop();
	}
	return 0;
}
