#pragma once

#include <Player.h>
#include <MY_ResourceManager.h>

#include <Box2DWorld.h>

Player::Player(Box2DWorld * _world, Shader * _shader) :
	Box2DSprite(_world, b2_dynamicBody, _shader, MY_ResourceManager::globalAssets->getTexture("body")->texture, 1, 1)
{
}

void Player::breakLimb(){
	limbs.back().segments.back()->body->SetType(b2_dynamicBody);
	for(auto s : limbs.back().joints){
		world->b2world->DestroyJoint(s);
	}
	limbs.pop_back();
}