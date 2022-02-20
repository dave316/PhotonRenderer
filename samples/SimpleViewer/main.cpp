#include "Application.h"
//#include <draco/mesh/mesh.h>
//#include <draco/compression/decode.h>
//#include <draco/core/decoder_buffer.h>
int main()
{
	//std::vector<unsigned char> compressedBuffer;
	//draco::DecoderBuffer dracoBuffer;
	//draco::Decoder decoder;
	//dracoBuffer.Init((char*)compressedBuffer.data(), compressedBuffer.size());
	//auto mesh = decoder.DecodeMeshFromBuffer(&dracoBuffer);

	Application app("PhotonRenderer", 1920, 1080);
	if (app.init())
	{
		app.loop();
	}
	return 0;
}