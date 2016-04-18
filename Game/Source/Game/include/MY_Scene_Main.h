#pragma once

#include <MY_Scene_Base.h>
#include <OpenALSound.h>
#include <sweet\UI.h>

class RenderSurface;
class StandardFrameBuffer;
class Bullet;

#define NUM_VERTS 64
#define REST_RAD 1.25f

class MY_Scene_Main : public MY_Scene_Base{
public:
	bool gameStarted, gameOver;

	NodeUI * startScreen, * endScreen;
	TextArea * endText;

	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	MeshInterface * meshThing;

	PerspectiveCamera * gameCam;

	std::vector<Bullet *> bullets;
	NodeUI * heart;
	float health;
	Timeout * heartbeat, * shoot, * hit;
	float heartBeatT;
	bool shooting;

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;
	
	glm::vec2 coords[NUM_VERTS];
	float damage[NUM_VERTS];


	void addBullet();

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	struct{
		float i;
		float dir;
		int offset;
		int randomness;
		float difficulty;
		int bulletsFired;
		int stagger;
	} enemy;
	float score;

	void reset();
};