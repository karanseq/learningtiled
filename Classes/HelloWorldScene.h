#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;

#define TILE_SIZE 32
#define GRAVITY -1
#define MOVEMENT_FORCE 2
#define MAX_VELOCITY_Y -32
#define MAX_VELOCITY_X 16

#define GET_X_FOR_COL(col) ( (col) * TILE_SIZE )
#define GET_Y_FOR_ROW(row, h) ( ( (h) - (row) ) * TILE_SIZE )
#define GET_COL_FOR_X(x) ( floor( (x) / TILE_SIZE ) )
#define GET_ROW_FOR_Y(y, h) ( (h) - ceil( (y) / TILE_SIZE ) )

enum ECollisionType
{
	E_COLLISION_NONE = 0,
	E_COLLISION_HERO_TOP,		// collision occurs above hero
	E_COLLISION_HERO_BOTTOM,	// collision occurs below hero
	E_COLLISION_HERO_LEFT,		// collision occurs to left of hero
	E_COLLISION_HERO_RIGHT,		// collision occurs to right of hero
};

class HelloWorld : public CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);
    
    // implement the "static node()" method manually
    CREATE_FUNC(HelloWorld);

	virtual void update(float dt);
	void UpdateHero();
	void UpdateForces();

	bool CheckCollisions();
	bool CheckVerticalCollisions(CCRect aabb);
	bool CheckHorizontalCollisions(CCRect aabb);
	void CollisionResponse(int tile_col, int tile_row, ECollisionType collision_type);
	void CheckConstraints();
	
	void RemoveBrick(int tile_col, int tile_row);

	virtual void ccTouchesBegan(CCSet* set, CCEvent* event);
	virtual void ccTouchesMoved(CCSet* set, CCEvent* event);
	virtual void ccTouchesEnded(CCSet* set, CCEvent* event);
	void HandleTouch(CCPoint touch_point, bool is_touching);

private:
	void CreateGame();
	void CreateMenu();

	CCTMXTiledMap* tiled_map_;
	CCTMXLayer* bricks_layer_;
	CCSprite* collidable_tile_;

	CCSprite* hero_;
	CCPoint speed_;
	CCPoint new_position_;
	bool is_hero_on_ground_;

	CCSprite* left_arrow_btn_;
	CCSprite* right_arrow_btn_;
	CCSprite* jump_btn_;
	bool is_left_arrow_pressed_;
	bool is_right_arrow_pressed_;
	bool is_jump_pressed_;

	CCSize screen_size_;
};

#endif // __HELLOWORLD_SCENE_H__
