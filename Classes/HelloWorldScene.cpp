#include "HelloWorldScene.h"

CCScene* HelloWorld::scene()
{
    CCScene *scene = CCScene::create();
    HelloWorld *layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

bool HelloWorld::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }
    
	tiled_map_ = NULL;
	bricks_layer_ = NULL;
	collidable_tile_ = NULL;
	hero_ = NULL;
	speed_ = CCPointZero;
	new_position_ = CCPointZero;
	is_hero_on_ground_ = false;
	left_arrow_btn_ = NULL;
	right_arrow_btn_ = NULL;
	jump_btn_ = NULL;
	is_left_arrow_pressed_ = false;
	is_right_arrow_pressed_ = false;
	is_jump_pressed_ = false;

	screen_size_ = CCDirector::sharedDirector()->getWinSize();

    CCMenuItemImage *pCloseItem = CCMenuItemImage::create("CloseNormal.png", "CloseSelected.png", this, menu_selector(HelloWorld::menuCloseCallback));    
	pCloseItem->setPosition(ccp(screen_size_.width - pCloseItem->getContentSize().width/2, screen_size_.height - pCloseItem->getContentSize().height/2));

    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);

    CreateGame();

    return true;
}

void HelloWorld::CreateGame()
{
	CCLayerColor* bg = CCLayerColor::create(ccc4(0, 25, 51, 255));
	addChild(bg);

	tiled_map_ = CCTMXTiledMap::create("tiledmap.tmx");
	addChild(tiled_map_);
	bricks_layer_ = tiled_map_->layerNamed("Bricks");

	hero_ = CCSprite::create("hero.png");
	hero_->setPosition(CCPoint(850.0f, 150.0f));
	tiled_map_->addChild(hero_);

	CreateMenu();

	setTouchEnabled(true);
	scheduleUpdate();
	//schedule(schedule_selector(HelloWorld::update), 1.0f);
}

void HelloWorld::CreateMenu()
{
	left_arrow_btn_ = CCSprite::create("left_arrow.png");
	right_arrow_btn_ = CCSprite::create("right_arrow.png");
	jump_btn_ = CCSprite::create("jump.png");

	left_arrow_btn_->setPosition(CCPoint(100.0f, 150.0f));
	right_arrow_btn_->setPosition(CCPoint(250.0f, 100.0f));
	jump_btn_->setPosition(CCPoint(1180.0f, 100.0f));

	addChild(left_arrow_btn_);
	addChild(right_arrow_btn_);
	addChild(jump_btn_);
}

void HelloWorld::update(float dt)
{
	UpdateHero();
}

void HelloWorld::UpdateHero()
{
	speed_.y += GRAVITY;
	speed_.y = (speed_.y < MAX_VELOCITY_Y) ? MAX_VELOCITY_Y : speed_.y;
	/*speed_.y = -MAX_VELOCITY_Y/2;
	speed_.x = -MAX_VELOCITY_X;*/

	UpdateForces();
	new_position_ = ccpAdd(hero_->boundingBox().origin, speed_);
	CheckCollisions();
	CheckConstraints();

	hero_->setPosition(ccpAdd(new_position_, ccp(hero_->getContentSize().width/2, hero_->getContentSize().height/2)));
}

void HelloWorld::UpdateForces()
{
	if(is_jump_pressed_ && is_hero_on_ground_)
	{
		speed_.y = TILE_SIZE;
		is_hero_on_ground_ = false;
	}

	if(is_left_arrow_pressed_)
	{
		speed_.x -= MOVEMENT_FORCE;
		speed_.x = (speed_.x < -MAX_VELOCITY_X) ? -MAX_VELOCITY_X : speed_.x;
	}

	if(is_right_arrow_pressed_)
	{
		speed_.x += MOVEMENT_FORCE;
		speed_.x = (speed_.x > MAX_VELOCITY_X) ? MAX_VELOCITY_X : speed_.x;
	}

	if(!is_left_arrow_pressed_ && !is_right_arrow_pressed_)
	{
		speed_.x -= speed_.x / 5;
	}
}

bool HelloWorld::CheckCollisions()
{
	CCRect hero_aabb = hero_->boundingBox();
	hero_aabb.origin = new_position_;
	
	CheckVerticalCollisions(hero_aabb);
	hero_aabb.origin = new_position_;
	CheckHorizontalCollisions(hero_aabb);

	return true;
}

bool HelloWorld::CheckVerticalCollisions(CCRect aabb)
{
	int visible_rows = (int)tiled_map_->getMapSize().height;
	int visible_cols = (int)tiled_map_->getMapSize().width;

	// since we're checking vertically, find the row occupied by the aabb
	int aabb_row = GET_ROW_FOR_Y(aabb.origin.y, visible_rows);
	if(speed_.y > 0)
	{
		// if we're going up, the top edge must be considered
		aabb_row = GET_ROW_FOR_Y(aabb.origin.y + aabb.size.height, visible_rows);
	}
	// also find the min & max edge of the aabb
	int aabb_start_col = GET_COL_FOR_X(aabb.origin.x);
	int aabb_end_col = GET_COL_FOR_X(aabb.origin.x + aabb.size.width);

	// bounds checking
	if(aabb_row < 0 || aabb_row >= visible_rows ||
			aabb_start_col < 0 || aabb_start_col >= visible_cols ||
			aabb_end_col < 0 || aabb_end_col >= visible_cols)
		return false;

	bool found_collidable = false;
	int current_col = aabb_start_col;
	int current_row = aabb_row;
	int current_tile = 0;

	while(current_row >= 0 && current_row < visible_rows)
	{
		// check for every column that the aabb occupies
		for(current_col = aabb_start_col; current_col <= aabb_end_col; ++current_col)
		{
			// calculate the index for a given tile
			current_tile  = current_col * visible_rows + current_row;

			//if(tile_data_[current_tile] != E_TILE_EMPTY && tile_data_[current_tile] != E_TILE_COLLECTIBLE && tile_data_[current_tile] != E_TILE_POWERUP)
			if(bricks_layer_->tileGIDAt(ccp(current_col, current_row)))
			{
				found_collidable = true;
				break;
			}
			/*else if(tile_data_[current_tile] == E_TILE_COLLECTIBLE || tile_data_[current_tile] == E_TILE_POWERUP)
			{
				if(player_->speed_.y < 0)
				{
					if(aabb.origin.y <= GET_Y_FOR_ROW(current_row + 1))
					{
						CollisionResponse(current_tile, current_col, current_row, E_COLLISION_DOWN);
					}
				}
				else
				{
					if((aabb.origin.y + aabb.size.height) >= GET_Y_FOR_ROW(current_row))
					{
						CollisionResponse(current_tile, current_col, current_row, E_COLLISION_UP);
					}
				}
			}*/
		}

		// from current tile, keep moving in same direction till a "collidable" tile is found
		if(found_collidable == false)
		{
			current_row = (speed_.y < 0) ? (current_row + 1):(current_row - 1);
		}
		else
		{
			break;
		}
	}

	if(found_collidable)
	{
		// going down
		if(speed_.y < 0)
		{
			// compare bottom edge of aabb with top edge of collidable row
			if(aabb.origin.y <= GET_Y_FOR_ROW(current_row, visible_rows))
			{
				CollisionResponse(current_col, current_row, E_COLLISION_HERO_BOTTOM);
			}
			else
			{
				found_collidable = false;
			}

			//collidable_tile_ = bricks_layer_->tileAt(ccp(current_col, current_row));
			//collidable_tile_->setColor(ccBLUE);
			//collidable_tile_->runAction(CCTintTo::create(0.5f, 255, 255, 255));
			//CCLOG("VERTICAL  COL:%d  ROW:%d  LHS:%f  RHS:%d", current_col, current_row, aabb.origin.y, GET_Y_FOR_ROW(current_row, visible_rows));
		}
		// going up
		else
		{
			// compare top edge of aabb with bottom edge of collidable row
			if((aabb.origin.y + aabb.size.height) >= GET_Y_FOR_ROW(current_row + 1, visible_rows))
			{
				CollisionResponse(current_col, current_row, E_COLLISION_HERO_TOP);
			}
			else
			{
				found_collidable = false;
			}

			//collidable_tile_ = bricks_layer_->tileAt(ccp(current_col, current_row));
			//collidable_tile_->setColor(ccBLUE);
			//collidable_tile_->runAction(CCTintTo::create(0.5f, 255, 255, 255));
			//CCLOG("VERTICAL  COL:%d  ROW:%d  LHS:%f  RHS:%d", current_col, current_row, aabb.origin.y, GET_Y_FOR_ROW(current_row + 1, visible_rows));
		}
	}

	return found_collidable;
}

bool HelloWorld::CheckHorizontalCollisions(CCRect aabb)
{
	int visible_rows = (int)tiled_map_->getMapSize().height;
	int visible_cols = (int)tiled_map_->getMapSize().width;

	// since we're checking horizontally, find the right facing column occupied by the aabb
	int aabb_col = GET_COL_FOR_X(aabb.origin.x + aabb.size.width);
	if(speed_.x < 0)
	{
		// if we're going left, the left facing column must be considered
		aabb_col = GET_COL_FOR_X(aabb.origin.x);
	}
	// also find the min & max edge of the aabb
	int aabb_start_row = GET_ROW_FOR_Y(aabb.origin.y + aabb.size.height, visible_rows);
	int aabb_end_row = GET_ROW_FOR_Y(aabb.origin.y, visible_rows);

	// bounds checking
	if(aabb_col < 0 || aabb_col >= visible_cols ||
			aabb_start_row < 0 || aabb_start_row >= visible_rows ||
			aabb_end_row < 0 || aabb_end_row >= visible_rows)
		return false;

	bool found_collidable = false;
	int current_col = aabb_col;
	int current_row = aabb_start_row;
	int current_tile = 0;

	while(current_col >= 0 && current_col < visible_cols)
	{
		// check for every row that the aabb occupies
		for(current_row = aabb_start_row; current_row <= aabb_end_row; ++current_row)
		{
			// calculate the index for a given tile
			current_tile  = current_col * visible_rows + current_row;

			//if(tile_data_[current_tile] != E_TILE_EMPTY && tile_data_[current_tile] != E_TILE_COLLECTIBLE && tile_data_[current_tile] != E_TILE_POWERUP)
			if(bricks_layer_->tileGIDAt(ccp(current_col, current_row)))
			{
				found_collidable = true;
				break;
			}
			/*else if(tile_data_[current_tile] == E_TILE_COLLECTIBLE || tile_data_[current_tile] == E_TILE_POWERUP)
			{
				if( (aabb.origin.x + aabb.size.width) >= GET_X_FOR_COL(current_col) )
				{
					CollisionResponse(current_tile, current_col, current_row, E_COLLISION_LEFT);
				}
			}*/
		}

		// from current tile, keep moving in same direction till a "collidable" tile is found
		if(found_collidable == false)
		{
			//current_col ++;
			current_col = (speed_.x < 0) ? (current_col - 1):(current_col + 1);
		}
		else
		{
			break;
		}
	}

	if(found_collidable)
	{
		if(speed_.x < 0)
		{
			// compare left edge of aabb with right edge of collidable column
			if( aabb.origin.x <= GET_X_FOR_COL(current_col + 1) )
			{
				CollisionResponse(current_col, current_row, E_COLLISION_HERO_LEFT);
			}
			else
			{
				found_collidable = false;
			}

			//collidable_tile_ = bricks_layer_->tileAt(ccp(current_col, current_row));
			//collidable_tile_->setColor(ccRED);
			//collidable_tile_->runAction(CCTintTo::create(0.5f, 255, 255, 255));
			//CCLOG("HORIZONTAL  COL:%d  ROW:%d  LHS:%f  RHS:%d", current_col, current_row, aabb.origin.x, GET_X_FOR_COL(current_col + 1));
		}
		// going right
		else
		{
			// compare right edge of aabb with left edge of collidable column
			if( (aabb.origin.x + aabb.size.width) >= GET_X_FOR_COL(current_col) )
			{
				CollisionResponse(current_col, current_row, E_COLLISION_HERO_RIGHT);
			}
			else
			{
				found_collidable = false;
			}

			//collidable_tile_ = bricks_layer_->tileAt(ccp(current_col, current_row));
			//collidable_tile_->setColor(ccRED);
			//collidable_tile_->runAction(CCTintTo::create(0.5f, 255, 255, 255));
			//CCLOG("HORIZONTAL  COL:%d  ROW:%d  LHS:%f  RHS:%d", current_col, current_row, aabb.origin.x + aabb.size.width, GET_X_FOR_COL(current_col));
		}
	}

	return found_collidable;
}

void HelloWorld::CollisionResponse(int tile_col, int tile_row, ECollisionType collision_type)
{
	switch(collision_type)
	{
	case E_COLLISION_HERO_TOP:
		speed_.y = 0;
		new_position_.y = GET_Y_FOR_ROW(tile_row + 1, tiled_map_->getMapSize().height) - hero_->getContentSize().height - 1;
		RemoveBrick(tile_col, tile_row);
		break;
	case E_COLLISION_HERO_BOTTOM:
		speed_.y = 0;
		new_position_.y = GET_Y_FOR_ROW(tile_row, tiled_map_->getMapSize().height) + 1;
		is_hero_on_ground_ = true;
		break;
	case E_COLLISION_HERO_LEFT:
		speed_.x = 0;
		new_position_.x = GET_X_FOR_COL(tile_col + 1) + 1;
		break;
	case E_COLLISION_HERO_RIGHT:
		speed_.x = 0;
		new_position_.x = GET_X_FOR_COL(tile_col) - hero_->getContentSize().width - 1;
		break;
	}

	//CCLOG("CollisionResponse tile_col:%d  tile_row:%d  collision_type:%d  new_position.x:%f  new_position.y:%f", tile_col, tile_row, collision_type, new_position_.x, new_position_.y);
}

void HelloWorld::CheckConstraints()
{
	if(new_position_.y < 0)
	{
		new_position_.y = 0;
		speed_.y = 0;
		is_hero_on_ground_ = true;
	}
	if(new_position_.x < 0)
	{
		new_position_.x = screen_size_.width;
	}
	if(new_position_.x > screen_size_.width)
	{
		new_position_.x = 0;
	}
}

void HelloWorld::RemoveBrick(int tile_col, int tile_row)
{
	bricks_layer_->removeTileAt(ccp(tile_col, tile_row));
}

void HelloWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	HandleTouch(touch_point, true);
}

void HelloWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	HandleTouch(touch_point, true);
}

void HelloWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	HandleTouch(touch_point, false);
}

void HelloWorld::HandleTouch(CCPoint touch_point, bool is_touching)
{
	is_left_arrow_pressed_ = left_arrow_btn_->boundingBox().containsPoint(touch_point) & is_touching;
	is_right_arrow_pressed_ = right_arrow_btn_->boundingBox().containsPoint(touch_point) & is_touching;
	is_jump_pressed_ = jump_btn_->boundingBox().containsPoint(touch_point) & is_touching;
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
	/*CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, HelloWorld::scene()));*/
	CCDirector::sharedDirector()->replaceScene(HelloWorld::scene());
}
