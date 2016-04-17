#pragma once

#include <Bullet.h>
#include <MY_ResourceManager.h>

Bullet::Bullet(Shader * _shader) :
	MeshEntity(MY_ResourceManager::globalAssets->getMesh("bullet")->meshes.at(0), _shader),
	r(5.f),
	reverse(false),
	hit(false)
{
	mesh->setScaleMode(GL_NEAREST);
}

void Bullet::update(Step * _step){
	r -= 0.05 * _step->deltaTimeCorrection * (reverse ? -1 : 1);
	firstParent()->translate(glm::cos(polar->x) * r, glm::sin(polar->x) * r, 0, false);

	MeshEntity::update(_step);
}