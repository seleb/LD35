#pragma once

#include <MY_Scene_ScreenShaders.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>

#include <NumberUtils.h>
#include <Easing.h>

MY_Scene_ScreenShaders::MY_Scene_ScreenShaders(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true))
{
	// set-up some UI to toggle between results
	//uiLayer->addMouseIndicator();
	sweet::setCursorMode(GLFW_CURSOR_NORMAL);


	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();


	meshThing = new TriMesh(true, GL_TRIANGLE_FAN);
	MeshEntity * m = new MeshEntity(meshThing, baseShader);
	childTransform->addChild(m);
	
	meshThing->pushVert(Vertex(0, 0, 0, 0, 0, 0, 1.f));

	for(unsigned long int i = 0; i < NUM_VERTS; ++i){
		coords[i].x = ((float)i/NUM_VERTS) * glm::pi<float>() * 2.f;// - glm::pi<float>()/2.f;
		coords[i].y = 1.f;

		meshThing->pushVert(Vertex(glm::cos(coords[i].x), glm::sin(coords[i].x), 0));
	}
	meshThing->indices.push_back(1);

	glLineWidth(1);

	gameCam = new PerspectiveCamera();
	gameCam->yaw = -90;
	gameCam->rotateVectors(gameCam->calcOrientation());
	cameras.push_back(gameCam);
	childTransform->addChild(gameCam)->translate(0,0,-5);
	activeCamera = gameCam;
	gameCam->interpolation = 1.f;
}

MY_Scene_ScreenShaders::~MY_Scene_ScreenShaders(){
	
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_ScreenShaders::update(Step * _step){
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}

	glm::vec2 sd = sweet::getWindowDimensions();

	glm::vec3 mousePos(mouse->mouseX()/sd.x - 0.5f, mouse->mouseY()/sd.y - 0.5f, 0.5);
	mousePos.z = 0;

	for(unsigned long int i = 0; i < NUM_VERTS; ++i){
		float g = glm::atan(mousePos.x, mousePos.y) + glm::pi<float>()/2.f;
		while(g < 0){
			g += glm::pi<float>() * 2.f;
		}while(g > glm::pi<float>() * 2.f){
			g -= glm::pi<float>() * 2.f;
		}
		float d = g - coords[i].x;
		while(d < -glm::pi<float>()){
			d += glm::pi<float>() * 2.f;
		}while(d > glm::pi<float>()){
			d -= glm::pi<float>() * 2.f;
		}
		d *= 0.1f;
		d /= glm::pi<float>()*2.f/NUM_VERTS;
		d = glm::min(1.f, glm::abs(d));
		float c = Easing::easeInOutCubic(d, 1, -1, 1);

		if(mouse->rightDown()){
			c *= -1;
		}else if(!mouse->leftDown()){
			c = 0;
		}

		
		coords[i].y += c*0.5f;
		coords[i].y = glm::clamp(coords[i].y, 0.5f, 2.f);
		coords[i].y += (1.25f - coords[i].y) * 0.1f;

		Vertex & v = meshThing->vertices.at(i+1);
		v.x = glm::cos(coords[i].x) * coords[i].y;
		v.y = glm::sin(coords[i].x) * coords[i].y;
		v.blue = v.red = v.green = coords[i].y/1.5f-0.5f;
		v.blue *= d;
		v.green *= d;

	}
	meshThing->dirty = true;


	// Scene update
	MY_Scene_Base::update(_step);
}

void MY_Scene_ScreenShaders::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	// keep our screen framebuffer up-to-date with the current viewport
	_renderOptions->setClearColour(0,0,0,0.5f);
	screenFBO->resize(_renderOptions->viewPortDimensions.width, _renderOptions->viewPortDimensions.height);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	glEnable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	MY_Scene_Base::render(_matrixStack, _renderOptions);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	//screenSurface->render(MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture->textureId);
	screenSurface->render(screenFBO->getTextureId(), false);

	// render the uiLayer after the screen surface in order to avoid hiding it through shader code
	uiLayer->render(_matrixStack, _renderOptions);
}

void MY_Scene_ScreenShaders::load(){
	MY_Scene_Base::load();	

	screenSurface->load();
	screenFBO->load();
}

void MY_Scene_ScreenShaders::unload(){
	screenFBO->unload();
	screenSurface->unload();

	MY_Scene_Base::unload();	
}