#pragma once

#include <MeshEntity.h>

#define BULLET_RAD 0.075f

class Bullet : public MeshEntity{
public:
	int idx;
	Vertex * v;
	glm::vec2 * polar;
	float r;
	bool reverse;
	bool hit;
	
	Bullet(Shader * _shader);

	virtual void update(Step * _step) override;
};