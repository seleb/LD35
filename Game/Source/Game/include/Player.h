#pragma once

#include <Box2DSprite.h>

struct Limb{
	std::vector<b2Joint *> joints;
	std::vector<Box2DSprite *> segments;
};

class Player : public Box2DSprite{
public:
	Player(Box2DWorld * _world, Shader * _shader);
	
	std::vector<Limb> limbs;

	void breakLimb();
};