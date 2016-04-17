#pragma once

#include <MY_Scene_ScreenShaders.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>

#include <NumberUtils.h>
#include <Easing.h>

#include <Bullet.h>
#include <AutoMusic.h>

MY_Scene_ScreenShaders::MY_Scene_ScreenShaders(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	health(1)
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
		coords[i].x = ((float)i/NUM_VERTS) * glm::pi<float>() * 2.f;
		coords[i].y = 1.f;
		damage[i] = 0;

		meshThing->pushVert(Vertex(0,0, 0));
	}
	meshThing->indices.push_back(1);

	glLineWidth(1.f);

	gameCam = new PerspectiveCamera();
	gameCam->yaw = -90;
	gameCam->rotateVectors(gameCam->calcOrientation());
	cameras.push_back(gameCam);
	childTransform->addChild(gameCam)->translate(0,0,-10);
	activeCamera = gameCam;
	gameCam->interpolation = 1.f;

	for(auto & v : MY_ResourceManager::globalAssets->getMesh("bullet")->meshes.at(0)->vertices){
		v.green = v.blue = 0;
	}

	
	NodeUI * overlay = new NodeUI(uiLayer->world);
	uiLayer->addChild(overlay);
	overlay->setRationalHeight(1.f, uiLayer);
	overlay->setSquareWidth(1.f);
	overlay->background->mesh->setScaleMode(GL_NEAREST);
	overlay->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("overlay")->texture);

	heart = new NodeUI(uiLayer->world);
	uiLayer->addChild(heart);
	heart->setRationalHeight(1.f, uiLayer);
	heart->setSquareWidth(1.f);
	heart->background->mesh->setScaleMode(GL_NEAREST);
	heart->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("heart")->texture);
	
	for(auto & v : heart->background->mesh->vertices){
		v.x -= 0.5;
		v.y -= 0.5;
	}
	heart->background->meshTransform->translate(0.5,0.5,0, false);

	MY_ResourceManager::globalAssets->getAudio("bgm")->sound->play(true);


	heartBeatT = 0;
	heartbeat = new Timeout(0.32f, [this](sweet::Event * _event){	
		health += 0.1f;
		if(health > 1){
			health = 1;
		}
		for(auto & d : damage){
			d -= 0.1f;
			if(d < 0){
				d = 0;
			}
		}
		heartbeat->restart();
		heartBeatT = 0;
	});
	heartbeat->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		heartBeatT = _event->getFloatData("progress");
	});
	
	childTransform->addChild(heartbeat, false);
	heartbeat->start();


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
	}test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "beat");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, heartBeatT);
		checkForGlError(0);
	}

	
#ifdef _DEBUG
	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}

	if(keyboard->keyJustDown(GLFW_KEY_R)){
		health = 1.f;
		for(auto & d : damage){
			d = 0.f;
		}
	}
#endif


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
		d *= 0.15f;
		d /= glm::pi<float>()*2.f/NUM_VERTS;
		d = glm::min(1.f, glm::abs(d));
		d = Easing::easeInOutCubic(d, 1, -1, 1);


		float target = REST_RAD*(1.f - damage[i]);

		if(mouse->rightDown()){
			target += (REST_RAD*0.25f - target) * d;
		}else if(mouse->leftDown()){
			target += (REST_RAD*3.f - target) * d;
		}

		
		//coords[i].y += c*0.5f;
		coords[i].y += (target - coords[i].y) * 0.1f;
		coords[i].y = glm::clamp(coords[i].y, REST_RAD*0.25f, REST_RAD*3.f);

		Vertex & v = meshThing->vertices.at(i+1);
		v.x = glm::cos(coords[i].x) * coords[i].y;
		v.y = glm::sin(coords[i].x) * coords[i].y;
		v.blue = 1.f - damage[i];
		v.green = (v.red + v.blue)*0.5f;
		//v.blue = v.red = v.green = coords[i].y/1.5f-0.5f;
		//v.blue = d;
		//v.green = d;

	}
	meshThing->dirty = true;





	if(_step->cycles % 6 == 0){
		addBullet();
	}

	for(signed long int i = bullets.size()-1; i >= 0; --i){
		Bullet * b = bullets.at(i);
		bool hit = b->r < b->polar->y + BULLET_RAD;
		bool destroy = false;
		if(hit){
			if(!b->reverse){
				if(b->polar->y > REST_RAD*1.25f){
					b->reverse = true;
					b->r = b->polar->y + BULLET_RAD;
				}else{
					if(!b->hit){
						b->hit = true;
						b->polar->y -= 0.5f;
						b->polar->y = glm::max(b->polar->y, 0.f);
						damage[b->idx] += 0.25f;
						// do a thing
						health -= 0.05f;
						if(damage[b->idx] > 1){
							damage[b->idx] = 1.f;
						}
					}
				}
			}
		}
		
		if(b->hit){
			if(b->r < 0){
				destroy = true;
			}
		}else if(b->reverse){
			if(b->r > 5.f){
				destroy = true;
			}
		}

		if( destroy ){
			bullets.erase(bullets.begin() + i);
			childTransform->removeChild(b->firstParent());
			delete b->firstParent();
		}
	}

	if(health < 0){
		health = 0;
	}
	heart->background->meshTransform->scale(health+heartBeatT*0.1f, false);


	// Scene update
	MY_Scene_Base::update(_step);
}

void MY_Scene_ScreenShaders::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	// keep our screen framebuffer up-to-date with the current viewport
	_renderOptions->setClearColour(0,0,0,0.15f);
	screenFBO->resize(_renderOptions->viewPortDimensions.width, _renderOptions->viewPortDimensions.height);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	glEnable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_renderOptions->clear();
	Scene::render(_matrixStack, _renderOptions);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	uiLayer->render(_matrixStack, _renderOptions);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	//screenSurface->render(MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture->textureId);
	screenSurface->render(screenFBO->getTextureId(), false);

	// render the uiLayer after the screen surface in order to avoid hiding it through shader code
	//uiLayer->render(_matrixStack, _renderOptions);
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

void MY_Scene_ScreenShaders::addBullet(){
	static int i = 1;
	static int dir = 1;
	static int offset = false;
	static int lastSample = 0;
	static int randomness = 1;
	static int difficulty = 1;

	bool shoot = MY_ResourceManager::globalAssets->getAudio("bgm")->sound->getAmplitude() > 0.01f;
	bool changeMode = (sweet::step.cycles % 60 <= difficulty) ? sweet::NumberUtils::randomBool() : false;

	difficulty = sweet::step.cycles / 1200;

	i += dir;
	i += sweet::NumberUtils::randomInt(-randomness,randomness);

	if(shoot){
		int idx = i;
		idx += offset;
		while(idx < 1){
			idx += NUM_VERTS;
		}while(idx > NUM_VERTS){
			idx -= NUM_VERTS;
		}

		Bullet * b = new Bullet(baseShader);
		b->idx = idx-1;
		b->v = &meshThing->vertices.at(idx);
		b->polar = &coords[idx-1];

		childTransform->addChild(b);
		b->firstParent()->translate(glm::cos(b->polar->x) * b->r, glm::sin(b->polar->x) * b->r, 0, false);

		bullets.push_back(b);
	}
		
	if(changeMode){
		dir = sweet::NumberUtils::randomInt(-difficulty, difficulty);
		offset = sweet::NumberUtils::randomInt(-NUM_VERTS/4, NUM_VERTS/4);
		randomness = sweet::NumberUtils::randomInt(0,difficulty);
	}
}