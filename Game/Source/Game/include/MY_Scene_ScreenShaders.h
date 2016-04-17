#pragma once

#include <MY_Scene_Base.h>

class RenderSurface;
class StandardFrameBuffer;

#define NUM_VERTS 256

class MY_Scene_ScreenShaders : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	MeshInterface * meshThing;

	PerspectiveCamera * gameCam;

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;

	glm::vec2 coords[NUM_VERTS];

	MY_Scene_ScreenShaders(Game * _game);
	~MY_Scene_ScreenShaders();
};