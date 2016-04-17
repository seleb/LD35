#pragma once

#include <MY_Scene_Base.h>
#include <OpenALSound.h>

class RenderSurface;
class StandardFrameBuffer;
class Bullet;

#define NUM_VERTS 64
#define REST_RAD 1.25f

class MY_Scene_ScreenShaders : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	MeshInterface * meshThing;

	PerspectiveCamera * gameCam;

	std::vector<Bullet *> bullets;
	NodeUI * heart;
	float health;
	Timeout * heartbeat;
	float heartBeatT;

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;
	
	glm::vec2 coords[NUM_VERTS];
	float damage[NUM_VERTS];


	void addBullet();

	MY_Scene_ScreenShaders(Game * _game);
	~MY_Scene_ScreenShaders();
};