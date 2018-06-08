#include "Thunder.h"
#include <algorithm>

USING_NS_CC;

using namespace CocosDenshion;

Scene* Thunder::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = Thunder::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool Thunder::init() {
	if (!Layer::init()) {
		return false;
	}
	stoneType = 0;
	isMove = false;  // 是否点击飞船
    ifGameOver = false;
	visibleSize = Director::getInstance()->getVisibleSize();

	// 创建背景
	auto bgsprite = Sprite::create("bg.jpg");
	bgsprite->setPosition(visibleSize / 2);
	bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width,
		visibleSize.height / bgsprite->getContentSize().height);
	this->addChild(bgsprite, 0);

	// 创建飞船
	player = Sprite::create("player.png");
	player->setAnchorPoint(Vec2(0.5, 0.5));
	player->setPosition(visibleSize.width / 2, player->getContentSize().height);
	player->setName("player");
	this->addChild(player, 1);

	// 显示陨石和子弹数量
	enemysNum = Label::createWithTTF("enemys: 0", "fonts/arial.TTF", 20);
	enemysNum->setColor(Color3B(255, 255, 255));
	enemysNum->setPosition(50, 60);
	this->addChild(enemysNum, 3);
	bulletsNum = Label::createWithTTF("bullets: 0", "fonts/arial.TTF", 20);
	bulletsNum->setColor(Color3B(255, 255, 255));
	bulletsNum->setPosition(50, 30);
	this->addChild(bulletsNum, 3);

	addEnemy(5);        // 初始化陨石
	preloadMusic();     // 预加载音乐
	playBgm();          // 播放背景音乐
	explosion();        // 创建爆炸帧动画

	// 添加监听器
	addTouchListener();
	addKeyboardListener();
	addCustomListener();

	// 调度器
	schedule(schedule_selector(Thunder::update), 0.04f, kRepeatForever, 0);

	return true;
}

//预加载音乐文件
void Thunder::preloadMusic() {
    auto audio = SimpleAudioEngine::getInstance();
    audio->preloadBackgroundMusic("music/bgm.mp3");
    audio->preloadEffect("music/explore.wav");
    audio->preloadEffect("music/fire.wav");
}

//播放背景音乐
void Thunder::playBgm() {
    SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bgm.mp3", true);
}

//初始化陨石
void Thunder::addEnemy(int n) {
	enemys.clear();
	for (unsigned i = 0; i < 3; ++i) {
		char enemyPath[20];
		sprintf(enemyPath, "stone%d.png", 3 - i);
		double width = visibleSize.width / (n + 1.0),
			height = visibleSize.height - (50 * (i + 1));
		for (int j = 0; j < n; ++j) {
			auto enemy = Sprite::create(enemyPath);
			enemy->setAnchorPoint(Vec2(0.5, 0.5));
			enemy->setScale(0.5, 0.5);
			enemy->setPosition(width * (j + 1), height);
			enemys.push_back(enemy);
			addChild(enemy, 1);
		}
	}
}

// 陨石向下移动并生成新的一行(加分项)
void Thunder::newEnemy() {
    for (auto enemy : enemys) {
        enemy->setPosition(enemy->getPosition().x, enemy->getPosition().y - 50);
    }
    char enemyPath[20];
    sprintf(enemyPath, "stone%d.png", stoneType+1);
    double width = visibleSize.width / (5 + 1.0),
        height = visibleSize.height - 50;
    for (int j = 0; j < 5; ++j) {
        auto enemy = Sprite::create(enemyPath);
        enemy->setAnchorPoint(Vec2(0.5, 0.5));
        enemy->setScale(0.5, 0.5);
        enemy->setPosition(width * (j + 0.5), height);
        enemys.push_back(enemy);
        addChild(enemy, 1);
    }
    if (++stoneType == 3) stoneType = 0;
}

// 移动飞船
void Thunder::movePlane(char c) {
	// Todo
    if (c == 'A' && player->getPosition().x >= 60) {
        MoveTo* move_to = MoveTo::create(0.1, Vec2(player->getPosition().x - 10, player->getPosition().y));
        player->runAction(move_to);
    }
    else if (c == 'D'&& player->getPosition().x <= visibleSize.width - 60) {
        MoveTo* move_to = MoveTo::create(0.1, Vec2(player->getPosition().x + 10, player->getPosition().y));
        player->runAction(move_to);
    }

}

//发射子弹
void Thunder::fire() {
	auto bullet = Sprite::create("bullet.png");
	bullet->setAnchorPoint(Vec2(0.5, 0.5));
	bullets.push_back(bullet);
	bullet->setPosition(player->getPosition());
	addChild(bullet, 1);
	SimpleAudioEngine::getInstance()->playEffect("music/fire.wav", false);

	// 移除飞出屏幕外的子弹
    float length = CCDirector::sharedDirector()->getWinSize().height + 
        bullet->getContentSize().height / 2 - bullet->getPosition().y;  //飞行距离，超出屏幕即结束
    float velocity = 420 / 1;                   //飞行速度：420pixel/sec
    float realMoveDuration = length / velocity; //飞行时间

    auto actionMove = MoveTo::create(realMoveDuration, ccp(bullet->getPosition().x, CCDirector::sharedDirector()->getWinSize().height + bullet->getContentSize().height / 2));
    auto cleanUp = CallFunc::create([bullet, this]() {
        this->bullets.remove(bullet);
        this->removeChild(bullet);
    });

    auto sequence = Sequence::create(actionMove, cleanUp, nullptr);
    bullet->runAction(sequence);
}

// 切割爆炸动画帧
void Thunder::explosion() {
	// Todo
    auto texture2 = Director::getInstance()->getTextureCache()->addImage("explosion.png");
    explore.reserve(7);
    for (int i = 0; i < 5; i++) {
        auto frame = SpriteFrame::createWithTexture(texture2, CC_RECT_PIXELS_TO_POINTS(Rect(190 * i, 0, 190, 200)));
        explore.pushBack(frame);
    }
    auto frame = SpriteFrame::createWithTexture(texture2, CC_RECT_PIXELS_TO_POINTS(Rect(190 * 1, 200, 190, 200)));
    explore.pushBack(frame);
    frame = SpriteFrame::createWithTexture(texture2, CC_RECT_PIXELS_TO_POINTS(Rect(190 * 2, 200, 190, 200)));
    explore.pushBack(frame);
}

void Thunder::update(float f) {
	// 实时更新页面内陨石和子弹数量(不得删除)
	// 要求数量显示正确(加分项)
	char str[15];
	sprintf(str, "enemys: %d", enemys.size());
	enemysNum->setString(str);
	sprintf(str, "bullets: %d", bullets.size());
	bulletsNum->setString(str);

	// 飞船移动
	if (isMove)
		this->movePlane(movekey);

	static int ct = 0;
	static int dir = 4;
	++ct;
	if (ct == 120)
		ct = 40, dir = -dir;
	else if (ct == 80) {
		dir = -dir;
		newEnemy();  // 陨石向下移动并生成新的一行(加分项)
	}
	else if (ct == 20)
		ct = 40, dir = -dir;

	//陨石左右移动
	for (Sprite* s : enemys) {
		if (s != NULL) {
			s->setPosition(s->getPosition() + Vec2(dir, 0));
		}
	}

	// 分发自定义事件
	EventCustom e("meet");
	_eventDispatcher->dispatchEvent(&e);

}

// 爆炸效果
void Thunder::Boom(Sprite* enemy) {
    auto exploreAnimation = Animation::createWithSpriteFrames(explore, 0.2f);
    auto exploreAnimate = Animate::create(exploreAnimation);
    auto cleanUp = CallFunc::create([enemy, this]() {
        this->enemys.remove(enemy);
        this->removeChild(enemy);
    });
    SimpleAudioEngine::getInstance()->playEffect("music/explore.wav");
    enemy->runAction(Sequence::create(exploreAnimate, cleanUp, nullptr));
}

// 自定义碰撞事件
void Thunder::meet(EventCustom * event) {
	// 判断子弹是否打中陨石并执行对应操作
    for (auto bullet : bullets) {
        for (auto enemy : enemys) {
            if (bullet->getPosition().getDistance(enemy->getPosition()) <= 20) {
                Boom(enemy);
                bullet->stopAllActions();
                this->removeChild(bullet);
                this->bullets.remove(bullet);
                break;
            }
        }
    }

    // 判断是否游戏结束，打到飞机或者越过x轴
    if (ifGameOver) return;
    for (auto enemy : enemys) {
        if (player->getPosition().getDistance(enemy->getPosition()) <= 20
            || enemy->getPosition().y < 0) {
            ifGameOver = true;
            break;
        }
    }
    if (ifGameOver) this->stopAc();

}

// 终止函数
void Thunder::stopAc() {
    auto exploreAnimation = Animation::createWithSpriteFrames(explore, 0.2f);
    auto exploreAnimate = Animate::create(exploreAnimation);
    auto signal = CallFunc::create([this]() {
        auto gameOver = Sprite::create("gameOver.png");
        gameOver->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        this->addChild(gameOver, 2);
        this->removeChild(this->player);
        CCDirector::sharedDirector()->pause();
    });
    SimpleAudioEngine::getInstance()->playEffect("music/explore.wav");
    auto seq = Sequence::create(exploreAnimate, signal, nullptr);
    player->runAction(seq);
}

// 添加自定义监听器
void Thunder::addCustomListener() {
	// Todo
    auto customListener = EventListenerCustom::create("meet", CC_CALLBACK_1(Thunder::meet, this));
    _eventDispatcher->addEventListenerWithSceneGraphPriority(customListener, this);
}

// 添加键盘事件监听器
void Thunder::addKeyboardListener() {
    auto keyBoardListener = EventListenerKeyboard::create(); 
    keyBoardListener->onKeyPressed = CC_CALLBACK_2(Thunder::onKeyPressed, this);
    keyBoardListener->onKeyReleased = CC_CALLBACK_2(Thunder::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyBoardListener, this);
}

void Thunder::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
	switch (code) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	case EventKeyboard::KeyCode::KEY_CAPITAL_A:
	case EventKeyboard::KeyCode::KEY_A:
		movekey = 'A';
		isMove = true;
		break;
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	case EventKeyboard::KeyCode::KEY_CAPITAL_D:
	case EventKeyboard::KeyCode::KEY_D:
		movekey = 'D';
		isMove = true;
		break;
	case EventKeyboard::KeyCode::KEY_SPACE:
		fire();
		break;
	}
}

void Thunder::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
	switch (code) {
	case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	case EventKeyboard::KeyCode::KEY_A:
	case EventKeyboard::KeyCode::KEY_CAPITAL_A:
	case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	case EventKeyboard::KeyCode::KEY_D:
	case EventKeyboard::KeyCode::KEY_CAPITAL_D:
		isMove = false;
		break;
	}
}

// 添加触摸事件监听器
void Thunder::addTouchListener() {
    auto TouchListener = EventListenerTouchOneByOne::create();
    TouchListener->onTouchBegan = CC_CALLBACK_2(Thunder::onTouchBegan, this);
    TouchListener->onTouchEnded = CC_CALLBACK_2(Thunder::onTouchEnded, this);
    TouchListener->onTouchMoved = CC_CALLBACK_2(Thunder::onTouchMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(TouchListener, this);
}

// 鼠标点击发射炮弹
bool Thunder::onTouchBegan(Touch *touch, Event *event) {
	if (touch->getLocation().getDistance(player->getPosition()) <= 30)
        isClick = true;
    
    //Todo
    if (isClick)
        fire();
    return isClick;
}

void Thunder::onTouchEnded(Touch *touch, Event *event) {
	isClick = false;
}

// 当鼠标按住飞船后可控制飞船移动 (加分项)
void Thunder::onTouchMoved(Touch *touch, Event *event) {
	// Todo
    if (touch->getLocation().x <= visibleSize.width - 60 && touch->getLocation().x >= 60)
        player->setPosition(touch->getLocation().x, player->getPosition().y);
}
