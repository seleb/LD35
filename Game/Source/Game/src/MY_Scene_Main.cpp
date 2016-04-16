#pragma once

#include <MY_Scene_Main.h>

#include <Box2DWorld.h>
#include <Box2DDebugDrawer.h>
#include <Box2DMeshEntity.h>
#include <Box2DSprite.h>

#include <Box2D\Dynamics\Joints\b2RevoluteJoint.h>

#include <MeshFactory.h>

#include <shader/ShaderComponentMVP.h>
#include <shader/ShaderComponentTexture.h>
#include <shader/ShaderComponentWorldSpaceUVs.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	box2dWorld(new Box2DWorld(b2Vec2(0.f, 0.0f))),
	box2dDebugDrawer(new Box2DDebugDrawer(box2dWorld))
{
	worldspaceShader = new ComponentShaderBase(true);
	worldspaceShader->addComponent(new ShaderComponentMVP(worldspaceShader));
	worldspaceShader->addComponent(new ShaderComponentTexture(worldspaceShader));
	worldspaceShader->addComponent(uvComponent = new ShaderComponentWorldSpaceUVs(worldspaceShader));
	uvComponent->xMultiplier = 0.05f;
	uvComponent->yMultiplier = 0.05f;
	//worldspaceShader->addComponent(new ShaderComponentDiffuse(worldspaceShader));
	worldspaceShader->compileShader();
	worldspaceShader->incrementReferenceCount();

	// Setup the debug drawer and add it to the scene
	childTransform->addChild(box2dDebugDrawer, false);
	box2dDebugDrawer->drawing = false;
	box2dWorld->b2world->SetDebugDraw(box2dDebugDrawer);
	box2dDebugDrawer->AppendFlags(b2Draw::e_shapeBit);
	box2dDebugDrawer->AppendFlags(b2Draw::e_centerOfMassBit);
	box2dDebugDrawer->AppendFlags(b2Draw::e_jointBit);

	// Setup the ground
	{
		ground = new Box2DSprite(box2dWorld, b2_staticBody, baseShader, MY_ResourceManager::globalAssets->getTexture("DEFAULT")->texture, 10, 1);
		b2Filter f;
		f.categoryBits = kGROUND;
		f.maskBits = kPLAYER;
		ground->createFixture(f); // when we're using a Box2DMeshEntity, createFixture will make a collider which matches the bounding box (or bounding circle, if you use the argument)
		childTransform->addChild(ground);
	}


	float d1 = 2, d2 = 1.f;

	// Setup the player
	player = new Box2DSprite(box2dWorld, b2_dynamicBody, baseShader, MY_ResourceManager::globalAssets->getTexture("body")->texture, 1, 1);
	player->mesh->setScaleMode(GL_NEAREST);
	player->meshTransform->scale(d1);
	//player->createFixture(); // when we're using a Box2DSprite, createFixture will make a collider which matches the provided width and height of the sprite (note that this is different from the actual texture size)
	
	b2CircleShape tShape;
	tShape.m_radius = d1*0.5f;
	
	b2FixtureDef fd;
	fd.shape = &tShape;
	fd.restitution = 0.f;
	fd.friction = 0.5f;
	fd.isSensor = false;
	fd.density = 1.f;
	fd.userData = nullptr;
	fd.filter = b2Filter();
	fd.filter.categoryBits = kPLAYER;
	fd.filter.maskBits = kGROUND;

	b2Fixture * f = player->body->CreateFixture(&fd);

	int numLimbs = 3;
	for(unsigned long int i = 0; i < numLimbs; ++i){
		float angle = (float)i/numLimbs * glm::pi<float>()*2.f;



		Box2DSprite * prev = player;
		int numSegments = 10;
		for(unsigned long int i = 0; i < numSegments; ++i){
			Box2DSprite * next = new Box2DSprite(box2dWorld, b2_dynamicBody, baseShader, MY_ResourceManager::globalAssets->getTexture("limb")->texture, d2, d2);
			next->meshTransform->scale(d2);
			if(i == numSegments-1){
				limbEnds.push_back(next);
				next->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("limbend")->texture);
			}

			next->mesh->setScaleMode(GL_NEAREST);
		
			b2CircleShape tShape;
			tShape.m_radius = d2*0.5f;
	
			b2FixtureDef fd;
			fd.shape = &tShape;
			fd.restitution = 0.f;
			fd.friction = 0.5f;
			fd.isSensor = false;
			fd.density = 1.f;
			fd.userData = nullptr;
			fd.filter = b2Filter();
			fd.filter.categoryBits = kLIMB;
			fd.filter.maskBits = 0;

			b2Fixture * f = next->body->CreateFixture(&fd);
		
		
			//next->createFixture(); // when we're using a Box2DSprite, createFixture will make a collider which matches the provided width and height of the sprite (note that this is different from the actual texture size)
			childTransform->addChild(next);

			b2RevoluteJointDef jointDef;
			jointDef.bodyA = prev->body;
			jointDef.bodyB = next->body;
			jointDef.collideConnected = false;
			jointDef.localAnchorA = b2Vec2(glm::cos(angle) * d1*0.25f, glm::sin(angle) * d1*0.25f);
			jointDef.localAnchorB = b2Vec2(glm::cos(angle) * d2*-0.25f, glm::sin(angle) * d2*-0.25f);
			box2dWorld->b2world->CreateJoint(&jointDef);
		
			prev = next;
			d1 = d2;
		}
	}

	// when dealing with physics nodes, we use translatePhysical instead of editing the Transform nodes directly
	// this is because we need to inform the physics simulation of the change, not our Transform hierarchy
	// the physics node will handle the placement of its childTransform automatically later during the update loop
	player->translatePhysical(glm::vec3(0, 6, 0), false); 
	
	childTransform->addChild(player);


	gameCam = new OrthographicCamera(-16, 16, -9, 9, -100, 100);
	cameras.push_back(gameCam);
	childTransform->addChild(gameCam);
	activeCamera = gameCam;
	
	MeshEntity * sky = new MeshEntity(MeshFactory::getPlaneMesh(), worldspaceShader);
	sky->mesh->setScaleMode(GL_NEAREST);
	sky->childTransform->rotate(90, 0,1,0, kOBJECT);
	sky->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("bg")->texture);
	gameCam->childTransform->addChild(sky)->scale(50);
	sky->firstParent()->translate(50,0,0);

	uiLayer->addMouseIndicator();
}

MY_Scene_Main::~MY_Scene_Main(){
	worldspaceShader->decrementAndDelete();

	// we need to destruct the scene elements before the physics world to avoid memory issues
	deleteChildTransform();


}


void MY_Scene_Main::update(Step * _step){
	// Physics update
	box2dWorld->update(_step);
	// Scene update
	MY_Scene_Base::update(_step);

	// player input
	/*player->applyLinearImpulseRight(controller->getAxis(controller->axisLeftX));
	if(controller->buttonJustDown(controller->faceButtonDown)){
		player->applyLinearImpulseUp(15);
	}*/

	glm::vec3 camPos = gameCam->childTransform->getWorldPos();
	glm::vec3 playerPos = player->getPhysicsBodyCenter();
	glm::vec3 camMovement = playerPos - camPos;
	camMovement.z = 0;

	gameCam->firstParent()->translate(camMovement * 0.1f);

	glm::vec2 sd = sweet::getWindowDimensions();

	glm::vec3 mousePos = activeCamera->screenToWorld(glm::vec3(mouse->mouseX()/sd.x, mouse->mouseY()/sd.y, 1), sd);

	
	if(mouse->leftJustPressed()){
		for(auto s : limbEnds){
			s->body->SetType(b2_staticBody);
			s->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("limbend-closed")->texture);
			//s->body->SetLinearDamping(1000);
		}
	}else if(mouse->leftJustReleased()){
		for(auto s : limbEnds){
			s->body->SetType(b2_dynamicBody);
			s->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("limbend")->texture);
			//s->body->SetLinearDamping(0);
		}
	}
	
	glm::vec3 bodyPos = player->getPhysicsBodyCenter();
	if(!mouse->leftDown()){
		for(auto s : limbEnds){
			glm::vec3 limbPos = s->getPhysicsBodyCenter();

			glm::vec3 d = mousePos - limbPos;
			d.z = 0;

			glm::vec3 d2 = bodyPos - limbPos;
			d2.z = 0;
			d2 /= 15.f;
			//d = glm::normalize(d);

			s->applyLinearImpulseToCenter(d/(float)limbEnds.size() * glm::length(d2));
		}
	}else{

		glm::vec3 d = mousePos - bodyPos;
		d.z = 0;
		//d = glm::normalize(d);

		player->applyLinearImpulseToCenter(d*0.5f);
	}
}

void MY_Scene_Main::enableDebug(){
	MY_Scene_Base::enableDebug();
	box2dDebugDrawer->drawing = true;
	childTransform->addChildAtIndex(box2dDebugDrawer, -1, false); // make sure the debug drawer is the last thing drawn
}
void MY_Scene_Main::disableDebug(){
	MY_Scene_Base::disableDebug();
	box2dDebugDrawer->drawing = false;
}