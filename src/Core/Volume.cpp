#include "Volume.h"

Volume::Volume(bool global, float distance) : 
	global(global),
	blendDistance(distance)
{

}

void Volume::setPPP(PostProcessParameters& params)
{
	this->ppParameters = params;
}

bool Volume::isGlobal()
{
	return global;
}

float Volume::getDistance()
{
	return blendDistance;
}

PostProcessParameters Volume::getParameters()
{
	return ppParameters;
}