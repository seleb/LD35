#pragma once

#include <Player.h>
#include <MY_ResourceManager.h>

Player::Player(Box2DWorld * _world, Shader * _shader) :
	Box2DSprite(_world, b2_dynamicBody, _shader, MY_ResourceManager::globalAssets->getTexture("body")->texture, 1, 1)
{
}