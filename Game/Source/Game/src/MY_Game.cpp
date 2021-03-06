#pragma once

#include <MY_Game.h>

#include <MY_ResourceManager.h>

#include <MY_Scene_Main.h>
#include <MY_Scene_Main.h>
/*#include <MY_Scene_Menu.h>
#include <MY_Scene_Box2D.h>
#include <MY_Scene_Bullet3D.h>
#include <MY_Scene_SurfaceShaders.h>
#include <MY_Scene_VR.h>*/


bool MY_Game::resized = false;

MY_Game::MY_Game() :
	Game("menu", new MY_Scene_Main(this), true), // initialize our game with a menu scene
	lastSize(0)
{
	// initialize all of our scenes in the game's scene map for later use
	// only the current scene is updated/rendered by default, so these shouldn't affect performance
	// however, they will be created at the same time as the game, so any load times associated with
	// their creation will be front-loaded. To avoid this, you can check to see if a scene exists immediately
	// before switching to it, and create it then if needed.
	/*scenes["box2d"] = new MY_Scene_Box2D(this);
	scenes["bullet3d"] = new MY_Scene_Bullet3D(this);
	scenes["screenshaders"] = new MY_Scene_Main(this);
	scenes["surfaceshaders"] = new MY_Scene_SurfaceShaders(this);
	scenes["vr"] = new MY_Scene_VR(this);*/
}

MY_Game::~MY_Game(){}

void MY_Game::addSplashes(){
	// add default splashes
	//Game::addSplashes();

	// add custom splashes
	//addSplash(new Scene_Splash(this, MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture, MY_ResourceManager::globalAssets->getAudio("DEFAULT")->sound));
}

void MY_Game::update(Step * _step){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int s = glm::min(sd.x, sd.y);
	if(sd.x != sd.y && !sweet::config.fullscreen){
		glfwSetWindowSize(sweet::currentContext, s, s);
	}
	if(s != lastSize){
		resized = true;
		lastSize = s;
	}
	Game::update(_step);
}