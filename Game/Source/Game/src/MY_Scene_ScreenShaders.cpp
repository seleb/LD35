#pragma once

#include <MY_Scene_ScreenShaders.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>

#include <NumberUtils.h>
#include <Easing.h>

#include <Bullet.h>
#include <AutoMusic.h>
#include <sweet\UI.h>

MY_Scene_ScreenShaders::MY_Scene_ScreenShaders(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	health(1),
	shooting(false),
	score(0)
{
	
	enemy.i = 1;
	enemy.dir = 1;
	enemy.offset = false;
	enemy.randomness = 1;
	enemy.difficulty = -1;
	enemy.bulletsFired = 0;
	enemy.stagger = 1;

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
		if(!mouse->leftDown()){
			health += 0.05f;
			if(health > 1){
				health = 1;
			}
			for(auto & d : damage){
				d -= 0.15f;
				if(d < 0){
					d = 0;
				}
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

	
	shoot = new Timeout(0.32f, [this](sweet::Event * _event){	
		shooting = !shooting;
		if(shooting){
			shoot->targetSeconds = sweet::NumberUtils::randomFloat(0.5f, 2.5f);
		}else{
			shoot->targetSeconds = sweet::NumberUtils::randomFloat(0.5f, 1.5f);
		}
		shoot->restart();
	});
	
	childTransform->addChild(shoot, false);
	shoot->start();

	
	hit = new Timeout(0.32f, [this](sweet::Event * _event){
		//gameCam->pitch = 0;
		//gameCam->yaw = -90;
		gameCam->roll = 0;
		MY_ResourceManager::globalAssets->getAudio("bgm")->sound->setPitch(1);
	});
	hit->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = 1.f-_event->getFloatData("progress");
		//gameCam->pitch = sweet::NumberUtils::randomFloat(-p,p)*15;
		//gameCam->yaw = -90;
		gameCam->roll = (sweet::NumberUtils::randomFloat(-p,p))*5;
		//MY_ResourceManager::globalAssets->getAudio("bgm")->sound->setPitch(1 + (1.f-sweet::NumberUtils::randomFloat(-p,p)));
	});
	
	childTransform->addChild(hit, false);
	

	MY_ResourceManager::globalAssets->getAudio("deflect")->sound->play(true);
	MY_ResourceManager::globalAssets->getAudio("deflect")->sound->setGain(0);

	MY_ResourceManager::globalAssets->getAudio("hit")->sound->play(true);
	MY_ResourceManager::globalAssets->getAudio("hit")->sound->setGain(0);


	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->background->setVisible(true);
	vl->setBackgroundColour(0.5,0.49,0,1);
	vl->marginBottom.setRationalSize(0.05f, &vl->height);
	vl->marginLeft.setRationalSize(0.05f, &vl->width);

	vl->setRenderMode(kTEXTURE);

	TextLabelControlled * txtScore = new TextLabelControlled(&score, 0, FLT_MAX, uiLayer->world, font, textShader);
	txtScore->prefix = "PTS: ";
	vl->addChild(txtScore);
	txtScore->setPadding(0.01f);
	TextLabelControlled * txtDifficulty = new TextLabelControlled(&enemy.difficulty, 0, FLT_MAX, uiLayer->world, font, textShader);
	txtDifficulty->prefix = "LVL: ";
	vl->addChild(txtDifficulty);
	txtDifficulty->setPadding(0.01f);
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
	MY_ResourceManager::globalAssets->getAudio("deflect")->sound->setGain(glm::max(0.f, MY_ResourceManager::globalAssets->getAudio("deflect")->sound->getGain(false)-0.1f));
	MY_ResourceManager::globalAssets->getAudio("hit")->sound->setGain(glm::max(0.f, MY_ResourceManager::globalAssets->getAudio("hit")->sound->getGain(false)-0.2f));


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
		d = Easing::easeInOutBounce(d, 1, -1, 1);


		float target = REST_RAD*(1.f - damage[i])*(health*0.75f+0.25f);

		/*if(mouse->rightDown()){
			target += (REST_RAD*0.25f - target) * d;
		}else */if(mouse->leftDown()){
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





	addBullet();

	for(signed long int i = bullets.size()-1; i >= 0; --i){
		Bullet * b = bullets.at(i);
		bool collision = b->r < b->polar->y + BULLET_RAD;
		bool destroy = false;
		if(collision){
			if(!b->reverse){
				if(b->polar->y > REST_RAD*1.25f){
					b->reverse = true;
					b->r = b->polar->y + BULLET_RAD;
					MY_ResourceManager::globalAssets->getAudio("deflect")->sound->setGain(1.f);
					MY_ResourceManager::globalAssets->getAudio("deflect")->sound->setPitch(pow(2,AutoMusic::scales[(b->idx%8)]/13.f));
					score += enemy.difficulty;
				}else{
					if(!b->hit){
						b->hit = true;
						b->polar->y -= 0.5f;
						b->polar->y = glm::max(b->polar->y, 0.f);
						damage[b->idx] += 0.3f;
						// do a thing
						health -= 0.01f;
						if(damage[b->idx] > 1){
							damage[b->idx] = 1.f;
						}
						hit->restart();
						MY_ResourceManager::globalAssets->getAudio("hit")->sound->setGain(1.f);
						MY_ResourceManager::globalAssets->getAudio("hit")->sound->setPitch(pow(2,AutoMusic::scales[(b->idx%8)]/13.f));
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
		MY_ResourceManager::globalAssets->getAudio("bgm")->sound->setPitch(1 + sweet::NumberUtils::randomFloat(-(1.f-health),(1.f-health)) );


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
	enemy.bulletsFired++;

	bool changeMode = (enemy.bulletsFired % 60 <= enemy.difficulty) ? sweet::NumberUtils::randomBool() : false;

	enemy.difficulty = glm::max(1.f, glm::floor(enemy.bulletsFired / 1200.f));

	enemy.i += enemy.dir/6.f;
	enemy.i += sweet::NumberUtils::randomInt(-enemy.randomness,enemy.randomness);

	if(shooting && enemy.bulletsFired % enemy.stagger == 0){
		int idx = enemy.i;
		idx += enemy.offset;
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
		enemy.dir = sweet::NumberUtils::randomFloat(-enemy.difficulty, enemy.difficulty);
		enemy.offset = sweet::NumberUtils::randomInt(-NUM_VERTS/4, NUM_VERTS/4);
		enemy.randomness = sweet::NumberUtils::randomInt(0, enemy.difficulty);
		if(enemy.dir == 0 && enemy.randomness == 0){
			if(sweet::NumberUtils::randomBool()){
				enemy.dir = sweet::NumberUtils::randomBool() ? -1 : 1;
			}else{
				enemy.randomness = 1;
			}
		}
		enemy.stagger = sweet::NumberUtils::randomInt(1,3);
	}
}