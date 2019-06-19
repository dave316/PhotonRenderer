#ifndef INCLUDED_COMPONENT
#define INCLUDED_COMPONENT

#pragma once

#include <memory>

class Component
{
public:
	virtual ~Component() = 0;
	typedef std::shared_ptr<Component> Ptr;
};

#endif // INCLUDED_COMPONENT