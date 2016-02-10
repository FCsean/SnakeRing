#include "HelloWorldScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
USING_NS_CC;

using namespace cocostudio::timeline;
using namespace std;


enum OnTile {wall, food, snake, empty, powerup};
enum Direction {up, down, left, right};
const int widthCount = 24;
const int heightCount = 19;
const int sideSize = 34;
const float TIMETILMOVE = .3F;

deque<Sprite*> snakeArray;
Direction towards = Direction::right;
float timeTilMove;
float currentTimeTilMove;
float minTimeTilMove = .05F;
Sprite* foodSprite;
OnTile tiles[widthCount][heightCount];
queue<EventKeyboard::KeyCode> userInput;
Label* scoreLabel;
Label* powerUpLabel;
int score;
bool gameOver;
int toAdd;
float timeTilSummonPowerup = 10.0F;
float currentTimeTilSummonPowerup;
float timeTilPowerupIsRemoved = 5.0F;
float currentTimeTilPowerupIsRemoved;
float timeTilPowerupIsDone = 6.0F;
float currentTimeTilPowerIsDone;
Sprite* powerupSprite;
boolean powerupInMap;
boolean powerupEaten;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}
void HelloWorld::menuCloseCallback(Ref* pSender)
{
	Director::getInstance()->end();
}
// on "init" you need to initialize your instance

auto createWall() {
	auto poringWall = Sprite::create("wall-poring.png");
	poringWall->setAnchorPoint(Vec2::ZERO);
	return poringWall;
}

auto createBody() {
	auto poringTail = Sprite::create("tail-poring.png");
	poringTail->setAnchorPoint(Vec2::ZERO);
	return poringTail;
}

Vec2 randomFood() {
	int x;
	int y;
	do {
		x = rand() % (widthCount - 2) + 1;
		y = rand() % (heightCount - 2) + 1;
	} while (tiles[x][y] != OnTile::empty);
	tiles[x][y] = OnTile::food;
	return Vec2(x * sideSize, y * sideSize);
}

Vec2 randomPowerup() {
	int x;
	int y;
	do {
		x = rand() % (widthCount - 2) + 1;
		y = rand() % (heightCount - 2) + 1;
	} while (tiles[x][y] != OnTile::empty);
	tiles[x][y] = OnTile::powerup;
	return Vec2(x * sideSize, y * sideSize);
}

void moveForward(Vec2 to, Vec2 from, HelloWorld* helloWorld) {
	Sprite* head = snakeArray.front();
	if (tiles[(int)to.x][(int)to.y] != OnTile::empty) {
		if (tiles[(int)to.x][(int)to.y] == OnTile::food) {
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("eat.wav");
			tiles[(int)to.x][(int)to.y] = snake;
			head->setPosition(Vec2(to.x*sideSize, to.y*sideSize));
			auto body = createBody();
			body->setPosition(Vec2(from.x*sideSize, from.y*sideSize));
			helloWorld->addChild(body);
			snakeArray.pop_front();
			snakeArray.push_front(body);
			snakeArray.push_front(head);
			foodSprite->setPosition(randomFood());
			timeTilMove -= 0.01F;
			if (timeTilMove < minTimeTilMove) {
				timeTilMove = minTimeTilMove;
			}
			score += toAdd;
			toAdd++;
			scoreLabel->setString(to_string(score));
		}
		else if (tiles[(int)to.x][(int)to.y] == OnTile::powerup) {
			powerupInMap = false;
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("decagi.wav");
			tiles[(int)to.x][(int)to.y] = snake;
			head->setPosition(Vec2(to.x*sideSize, to.y*sideSize));
			auto body = createBody();
			body->setPosition(Vec2(from.x*sideSize, from.y*sideSize));
			helloWorld->addChild(body);
			snakeArray.pop_front();
			snakeArray.push_front(body);
			snakeArray.push_front(head);
			powerupEaten = true;
			currentTimeTilPowerIsDone = timeTilPowerupIsDone;
			powerupSprite->setPosition(-34, -34);
			powerUpLabel->setString("5");
		}
		else {
			helloWorld->unscheduleUpdate();
			auto gameOverLabel = Label::createWithTTF("GAME OVER", "micross.ttf", 100);
			// position the label on the center of the screen
			gameOverLabel->setColor(Color3B::YELLOW);
			gameOverLabel->setPosition(Vec2(widthCount*sideSize / 2, heightCount*sideSize / 2));
			helloWorld->addChild(gameOverLabel, 1);
			gameOver = true;
			auto restartLabel = Label::createWithTTF("Press space to Restart", "micross.ttf", 50);
			// position the label on the center of the screen
			restartLabel->setColor(Color3B::YELLOW);
			restartLabel->setPosition(Vec2(widthCount*sideSize / 2, heightCount*sideSize / 2 - gameOverLabel->getBoundingBox().size.height));
			helloWorld->addChild(restartLabel, 1);

		}
	}
	else {
		tiles[(int)to.x][(int)to.y] = snake;
		head->setPosition(Vec2(to.x*sideSize, to.y*sideSize));
		Sprite* tail = snakeArray.back();
		Vec2 tailPos = tail->getPosition();
		tiles[(int)(tailPos.x/sideSize)][(int)(tailPos.y / sideSize)] = OnTile::empty;
		tail->setPosition(Vec2(from.x*sideSize, from.y*sideSize));
		snakeArray.pop_back();
		snakeArray.pop_front();
		snakeArray.push_front(tail);
		snakeArray.push_front(head);
	}
}

void move(HelloWorld* helloWorld) {
	Sprite* head = snakeArray.front();
	Vec2 position = head->getPosition();
	int x = position.x / sideSize;
	int y = position.y / sideSize;
	switch (towards) {
	case up:
		moveForward(Vec2(x, y + 1), Vec2(x, y), helloWorld);
		break;
	case Direction::left:
		moveForward(Vec2(x - 1, y), Vec2(x, y), helloWorld);
		break;
	case down:
		moveForward(Vec2(x, y - 1), Vec2(x, y), helloWorld);
		break;
	case Direction::right:
		moveForward(Vec2(x + 1, y), Vec2(x, y), helloWorld);
		break;
	}
}

bool HelloWorld::init()
{
    //  you can create scene with following comment code instead of using csb file.
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
	auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	audio->preloadBackgroundMusic("bgm.mp3");
	audio->playBackgroundMusic("bgm.mp3", true);
	audio->preloadEffect("eat.wav");
	audio->preloadEffect("decagi.wav");
	audio->preloadEffect("incagi.wav");
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
	auto gameLabel = Label::createWithTTF("Snakering", "micross.ttf", 100);
	// position the label on the center of the screen
	gameLabel->setColor(Color3B::YELLOW);
	gameLabel->setPosition(Vec2(widthCount*sideSize / 2, heightCount*sideSize / 2));
	this->addChild(gameLabel, 1);
	auto startLabel = Label::createWithTTF("Press space to start", "micross.ttf", 50);
	// position the label on the center of the screen
	startLabel->setColor(Color3B::YELLOW);
	startLabel->setPosition(Vec2(widthCount*sideSize / 2, heightCount*sideSize / 2 - gameLabel->getBoundingBox().size.height));
	this->addChild(startLabel, 1);
	gameOver = true;
	auto eventListener = EventListenerKeyboard::create();
	eventListener->onKeyPressed = [this, audio](EventKeyboard::KeyCode keyCode, Event* event) {
		if (keyCode == EventKeyboard::KeyCode::KEY_SPACE && gameOver) {
			this->removeAllChildren();
			this->initWorld();
		}
		else if (keyCode == EventKeyboard::KeyCode::KEY_P) {
			if (audio->isBackgroundMusicPlaying())
				audio->pauseBackgroundMusic();
			else
				audio->resumeBackgroundMusic();
		}
		else {
			userInput.push(keyCode);
		}
	};
	this->_eventDispatcher->addEventListenerWithFixedPriority(eventListener, 1);
    return true;
}

void HelloWorld::initWorld() {
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	gameOver = false;
	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	//   auto closeItem = MenuItemImage::create(
	//                                          "CloseNormal.png",
	//                                          "CloseSelected.png",
	//                                          CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
	//   
	//closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
	//                               origin.y + closeItem->getContentSize().height/2));

	//   // create menu, it's an autorelease object
	//   auto menu = Menu::create(closeItem, NULL);
	//   menu->setPosition(Vec2::ZERO);
	//   this->addChild(menu, 1);

	/////////////////////////////
	// 3. add your codes below...

	while (!snakeArray.empty())
		snakeArray.pop_back();
	towards = Direction::right;
	timeTilMove = TIMETILMOVE;
	while (!userInput.empty())
		userInput.pop();
	score = 0;
	toAdd = 1;
	currentTimeTilSummonPowerup = timeTilSummonPowerup;
	currentTimeTilPowerupIsRemoved = timeTilPowerupIsRemoved;
	currentTimeTilPowerIsDone = timeTilPowerupIsDone;
	powerupInMap = false;
	powerupEaten = false;
	powerupSprite = Sprite::create("powerup.png");
	powerupSprite->setAnchorPoint(Vec2::ZERO);
	powerupSprite->setPosition(-34, -34);
	this->addChild(powerupSprite, 1);
	scoreLabel = Label::createWithTTF("0", "micross.ttf", 24);
	scoreLabel->setAnchorPoint(Vec2::ZERO);
	// position the label on the center of the screen
	scoreLabel->setColor(Color3B::WHITE);
	scoreLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
		646));

	this->addChild(scoreLabel, 1);
	powerUpLabel = Label::createWithTTF("", "micross.ttf", 24);
	powerUpLabel->setAnchorPoint(Vec2::ZERO);
	powerUpLabel->setColor(Color3B::WHITE);
	powerUpLabel->setPosition(Vec2(0, 646));
	this->addChild(powerUpLabel, 1);

	auto background = Sprite::create("background.png");

	background->setAnchorPoint(Vec2::ZERO);
	background->setPosition(Vec2::ZERO);
	this->addChild(background, 0);

	for (int i = 0; i < widthCount; i++) {
		for (int j = 0; j < heightCount; j++) {
			tiles[i][j] = OnTile::empty;
		}
	}

	//Create walls
	for (int i = 0; i < widthCount; i++) {
		tiles[i][0] = wall;
		auto poringWall = createWall();
		poringWall->setPosition(Vec2(i*sideSize, 0));
		this->addChild(poringWall, 0);
	}

	for (int i = 0; i < widthCount; i++) {
		tiles[i][heightCount - 1] = wall;
		auto poringWall = createWall();
		poringWall->setPosition(Vec2(i*sideSize, (heightCount - 1) * sideSize));
		this->addChild(poringWall, 0);
	}

	for (int i = 1; i < heightCount - 1; i++) {
		tiles[0][i] = wall;
		auto poringWall = createWall();
		poringWall->setPosition(Vec2(0, i*sideSize));
		this->addChild(poringWall, 0);
	}

	for (int i = 1; i < heightCount - 1; i++) {
		tiles[widthCount - 1][i] = wall;
		auto poringWall = createWall();
		poringWall->setPosition(Vec2((widthCount - 1) * sideSize, i*sideSize));
		this->addChild(poringWall, 0);
	}

	auto head = Sprite::create("right-head-poring.png");
	head->setAnchorPoint(Vec2::ZERO);
	auto mid = createBody();
	auto tail = createBody();
	head->setPosition(Vec2(4 * sideSize, 9 * sideSize));
	mid->setPosition(Vec2(3 * sideSize, 9 * sideSize));
	tail->setPosition(Vec2(2 * sideSize, 9 * sideSize));

	this->addChild(head);
	this->addChild(mid);
	this->addChild(tail);


	tiles[4][9] = snake;
	tiles[3][9] = snake;
	tiles[2][9] = snake;

	towards = Direction::right;

	snakeArray.push_front(head);
	snakeArray.push_back(mid);
	snakeArray.push_back(tail);

	currentTimeTilMove = timeTilMove;

	foodSprite = Sprite::create("food.png");
	foodSprite->setAnchorPoint(Vec2::ZERO);
	foodSprite->setPosition(randomFood());
	this->addChild(foodSprite);
	this->scheduleUpdate();
	
	auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	
}

void HelloWorld::update(float delta) {
	if (currentTimeTilMove < 0) {
		bool moved = false;
		while (!moved && !userInput.empty()) {
			switch (userInput.front()) {
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			case EventKeyboard::KeyCode::KEY_A:
				switch (towards) {
				case up: case down:
					towards = Direction::left;
					snakeArray.front()->setTexture("left-head-poring.png");
					moved = true;
				}
				break;
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			case EventKeyboard::KeyCode::KEY_D:
				switch (towards) {
				case up: case down:
					towards = Direction::right;
					snakeArray.front()->setTexture("right-head-poring.png");
					moved = true;
				}
				break;
			case EventKeyboard::KeyCode::KEY_UP_ARROW:
			case EventKeyboard::KeyCode::KEY_W:
				switch (towards) {
				case Direction::right: case Direction::left:
					towards = Direction::up;
					moved = true;
				}
				break;
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			case EventKeyboard::KeyCode::KEY_S:
				switch (towards) {
				case Direction::right: case Direction::left:
					towards = Direction::down;
					moved = true;
				}
				break;

			}
			userInput.pop();
		}
		move(this);
		currentTimeTilMove += (powerupEaten ? 2 : 1) * timeTilMove;
	}
	else {
		currentTimeTilMove -= delta;
	}
	currentTimeTilSummonPowerup -= delta;
	if (currentTimeTilSummonPowerup < 0) {
		powerupSprite->setPosition(randomPowerup());
		currentTimeTilSummonPowerup = timeTilSummonPowerup + timeTilPowerupIsRemoved;
		currentTimeTilPowerupIsRemoved = timeTilPowerupIsRemoved;
		powerupInMap = true;
	}
	currentTimeTilPowerupIsRemoved -= delta;
	if (powerupInMap && currentTimeTilPowerupIsRemoved < 0) {
		powerupInMap = false;
		Vec2 position = powerupSprite->getPosition();
		tiles[(int)(position.x/sideSize)][(int)(position.y/sideSize)] = OnTile::empty;
		powerupSprite->setPosition(-34, -34);
	}
	currentTimeTilPowerIsDone -= delta;
	if (powerupEaten) {
		powerUpLabel->setString(std::to_string(currentTimeTilPowerIsDone));
	}
	if (powerupEaten && currentTimeTilPowerIsDone < 0) {
		powerupEaten = false;
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("incagi.wav");
		powerUpLabel->setString("");
	}
}