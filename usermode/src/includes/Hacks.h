#pragma once
#include "includes.h"
#include "Entity.h" //This needs to be here for some odd reason, delete it and find out!

class Hacks
{
public:
	Hacks(const Memory& memory, const Entity& entity) : mem(memory), ent(entity)
	{
	}
	~Hacks() = default;
	void hack_loop();

private:
	Memory mem;
	Entity ent;
};

