#pragma once

#include <MY_Scene_Base.h>

class Box2DWorld;
class Box2DDebugDrawer;
class Box2DSprite;
class Box2DMeshEntity;
class Player;

typedef enum{
	kGROUND = (1 << 1),
	kPLAYER = (1 << 2),
	kLIMB = (1 << 3)
} Box2D_Types;


class ShaderComponentWorldSpaceUVs;

// A sample scene showing some of the basics of integrating a Box2D physics simulation into a game scene
class MY_Scene_Main : public MY_Scene_Base{
public:
	ComponentShaderBase * worldspaceShader;
	ShaderComponentWorldSpaceUVs * uvComponent;


	// The scene's physics world
	Box2DWorld * box2dWorld;
	// used to draw wireframes showing physics colliders, transforms, etc
	Box2DDebugDrawer * box2dDebugDrawer;

	
	// A Box2DMeshSprite is a class which combines a Box2D physics node and a standard Sprite.
	// The physics node alters the position/orientation of the Sprite's childTransform
	// during the update loop to match the physics body's position/orientation within the Box2D world
	//
	// In this scene, we are using a Box2DSprite to represent a player object
	// In order to control the player object, we will be treating it as a rigid body and applying forces/impulses
	Player * player;
	std::vector<Box2DSprite * > limbEnds;
	// Similar to the Box2DMeshSprite, Box2DMeshEntity is a class which combines a Box2D physics node and a standard MeshEntity
	//
	// In this scene, we are using a Box2DMeshEntity to represent a ground object
	// This object will be a static body
	Box2DSprite * ground;

	OrthographicCamera * gameCam;


	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	virtual void update(Step * _step) override;
	
	// overriden to add physics debug drawing
	virtual void enableDebug() override;
	// overriden to remove physics debug drawing
	virtual void disableDebug() override;
};