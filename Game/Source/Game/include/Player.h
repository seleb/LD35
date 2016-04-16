#pragma once

#include <Box2DSprite.h>

class Player : public Box2DSprite{
public:
	Player(Box2DWorld * _world, Shader * _shader);

};