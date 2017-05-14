/*
Name: Matthew Persad
NetID: mp3685
Final Project

Basic demo for a platformer game.
Controls:
'A' is to move left, 'D' is to move right, and 'W' is to jump.
'Enter' or the return key is to shoot.
You can press the 'R' button to restart the level.
You can also press the 'ESC' key to exit the game.

Your goal is to collect all of the coins in the level,
while trying to avoid getting shot by the enemies.

HAVE FUN!
*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

#define TILE_SIZE 0.1f
#define LEVEL_HEIGHT 16
#define LEVEL_WIDTH 128
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define MAX_BULLETS 50
#define MAX_COINS 11
#define MAX_ENEMIES 10

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define "NYUCodebase.app/Contents/Resources/"
#endif

//main game variables
SDL_Window* displayWindow;

Matrix projectionMatrix;
Matrix viewMatrix;

//function to load a texture
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return retTexture;
}

//function to draw the image
void drawImage(ShaderProgram& program, Matrix modelMatrix, Matrix projectionMatrix, Matrix viewMatrix) {
	program.setModelMatrix(modelMatrix);
	program.setProjectionMatrix(projectionMatrix);
	program.setViewMatrix(viewMatrix);

	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

//linear interpolation function
float lerp(float from, float to, float time) {
	return (1.0 - time)*from + time*to;
}

//Color class
class Color {
public:
	float r;
	float g;
	float b;
	float a;
};

//Text class
class Text {
public:
	Text() {}
	Text(unsigned int texture) {
		textureID = texture;
	}

	void Draw(ShaderProgram& program, int character, float x, float y, bool test = false, bool test2 = false) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		float u = (float)(((int)character) % 16) / (float)16;
		float v = (float)(((int)character) / 16) / (float)16;
		float width = 1.0 / (float)16;
		float height = 1.0 / (float)16;
		GLfloat texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height };
		float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

		modelMatrix.identity();
		modelMatrix.Translate(x, y, 0.0f);
		if (test) { modelMatrix.Scale(0.125f, 0.125f, 1.0f); }
		else { modelMatrix.Scale(0.25f, 0.25f, 1.0f); }
		if (test2) { modelMatrix.Scale(0.75f, 0.75f, 1.0f); }

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

	GLuint textureID;
	Matrix modelMatrix;
	map<char, int> ascii;
};

//Vector3 class
class Vector3 {
public:
	Vector3() {}
	Vector3(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}

	bool operator==(Vector3& test) {
		return (this->x == test.x && this->y == test.y && this->z == test.z);
	}

	float x;
	float y;
	float z;
};

//Sprite Sheet class
class SheetSprite {
public:
	SheetSprite() {}
	SheetSprite(unsigned int textureID_) : textureID(textureID_) {}

	void Draw(ShaderProgram& program, int character, Vector3 position, Vector3 size) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		float u = (float)(((int)character) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
		float v = (float)(((int)character) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
		float width = 1.0 / (float)SPRITE_COUNT_X;
		float height = 1.0 / (float)SPRITE_COUNT_Y;

		GLfloat texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height };

		float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

		modelMatrix.identity();
		modelMatrix.Translate(position.x, position.y, 0.0f);
		modelMatrix.Scale(size.x, size.y, 1.0f);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

	float size = 0.2;
	unsigned int textureID;
	Matrix modelMatrix;
};

//Entity class
class Entity {
public:
	Entity() {}
	Entity(SheetSprite& sprite_) : sprite(sprite_) {}

	string name() {
		return id;
	}

	void Draw(ShaderProgram& program, int index) {
		sprite.Draw(program, index, position, size);
	}

	void Draw(ShaderProgram& program) {
		modelMatrix.identity();
		modelMatrix.Translate(position.x, position.y, 0.0f);
		modelMatrix.Scale(size.x, size.y, 1.0f);

		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}

	bool operator==(Entity& test) {
		return (this->position == test.position && this->velocity == test.velocity && this->acceleration == test.acceleration && this->size == test.size);
	}

	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 friction;
	Vector3 size;
	SheetSprite sprite;
	Matrix modelMatrix;
	int moving;
	bool collidedLeft;
	bool collidedRight;
	bool collidedTop;
	bool collidedBottom;
	string id;
	bool air;
	bool active;
};

//Player class
class Player : public Entity {
public:
	Player() {}
	Player(SheetSprite& sprite_) : Entity(sprite_) {}

	string name() const {
		return typeid(*this).name();
	}

	void Draw(ShaderProgram& program, int index) {
		Entity::Draw(program, index);
	}
	
	void Draw(ShaderProgram& program) {
		Entity::Draw(program);
	}

	Vector3 base;
};

//Player Legs class
class Legs : public Player {
public:
	Legs() {}
	Legs(SheetSprite& sprite_) : Player(sprite_) {}

	string name() const {
		return typeid(*this).name();
	}

	void Draw(ShaderProgram& program, int index) {
		Player::Draw(program, index);
	}

	void Draw(ShaderProgram& program) {
		Player::Draw(program);
	}

	int index;
};

//Enemy class
class Enemy : public Entity {
public:
	Enemy() {}
	Enemy(SheetSprite& sprite_) : Entity(sprite_) {}

	string name() const {
		return typeid(*this).name();
	}

	void Draw(ShaderProgram& program, int index) {
		Entity::Draw(program, index);
	}

	void Draw(ShaderProgram& program) {
		Entity::Draw(program);
	}

	int state;
	Vector3 base;
	int count;
	float shoot_time;
};

//Coin class
class Coin : public Entity {
public:
	Coin() {}
	Coin(SheetSprite& sprite_) : Entity(sprite_) {}

	string name() const {
		return typeid(*this).name();
	}

	void Draw(ShaderProgram& program, int index) {
		Entity::Draw(program, index);
	}

	void Draw(ShaderProgram& program) {
		Entity::Draw(program);
	}

	int index;
};

//Bullet class
class Bullet : public Entity {
public:
	Bullet() {
		position.y = -2000.0f;
		active = false;
		size.x = 0.05f;
		size.y = 0.05f;
		lifetime = 0.0f;
	}
	Bullet(SheetSprite& sprite_) : Entity(sprite_) {}

	void Draw(ShaderProgram& program, int index) {
		Entity::Draw(program, index);
	}

	void Draw(ShaderProgram& program) {
		Entity::Draw(program);
	}

	float lifetime;
	int direction;
};

//Particle class
class Particle {
public:
	Vector3 position;
	Vector3 velocity;
	float lifetime;
	float original;
};

//Particle Emitter class
class ParticleEmitter {
public:
	~ParticleEmitter() {
		particles.clear();
	}

	void Render(ShaderProgram& program, float size = 2.5f) {
		vector<float> vertices;
		for (int i = 0; i < particles.size(); i++) {
			vertices.push_back(particles[i].position.x);
			vertices.push_back(particles[i].position.y);
		}
		program.setModelMatrix(m);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glPointSize(size);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
		glEnableVertexAttribArray(program.positionAttribute);

		GLuint colorAttribute = glGetAttribLocation(program.programID, "color");

		std::vector<float> particleColors;
		for (int i = 0; i < particles.size(); i++) {
			particleColors.push_back(start.r);
			particleColors.push_back(start.g);
			particleColors.push_back(start.b);
			particleColors.push_back(start.a);
		}

		glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, false, 0, particleColors.data());
		glEnableVertexAttribArray(colorAttribute);

		glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	Vector3 position;
	float maxLifetime;
	std::vector<Particle> particles;
	Matrix m;
	Color start;
	Color end;
};

//Snow class
class Snow : public ParticleEmitter {
public:
	Snow(unsigned int particleCount) {
		position.x = 4.0f;
		position.y = 1.0f;
		gravity.y = -0.25f;
		maxLifetime = 5.0f;

		start.r = 1.0f;
		start.g = 1.0f;
		start.b = 1.0f;
		start.a = 1.0f;
		end.r = 1.0f;
		end.g = 1.0f;
		end.b = 1.0f;
		end.a = 1.0f;

		for (size_t index = 0; index < particleCount; ++index) {
			Particle temp;
			temp.position.x = (float)((float)rand() / ((float)RAND_MAX + 1) * 5);
			if (rand() % 2) {
				temp.position.x *= -1;
			}
			temp.position.x -= 0.625f - position.x;
			temp.original = temp.position.x;
			temp.position.y = 4 + (float)((float)rand() / ((float)RAND_MAX + 1) * 1);
			temp.lifetime = (float)((float)rand() / ((float)RAND_MAX + 1) * 5);
			temp.velocity.x = 0.0125f / 4;
			temp.velocity.y = 0.0f;
			particles.push_back(temp);
		}
	}

	~Snow() {
		ParticleEmitter::~ParticleEmitter();
	}

	void Update(float elapsed) {
		for (size_t index = 0; index < particles.size(); ++index) {
			if (particles[index].lifetime >= maxLifetime) {
				particles[index].position.x = particles[index].original;
				particles[index].position.y = position.y;
				particles[index].velocity.y = 0.0f;
				particles[index].lifetime = (maxLifetime - particles[index].lifetime) / maxLifetime;
			}
			else {
				particles[index].position.x += particles[index].velocity.x;
				particles[index].velocity.y += gravity.y * elapsed;
				particles[index].position.y += particles[index].velocity.y * elapsed;
				particles[index].lifetime += elapsed;
			}
		}
	}

	void Render(ShaderProgram& program) {
		ParticleEmitter::Render(program);
	}

	Vector3 gravity;
};

//necessary variables for the game
Snow snow(1000);

Player player;
Legs player_legs;
vector<Enemy> enemies;
vector<Coin> coins;
Entity temp;

bool left_ = false;
bool right_ = false;
bool last_moved_l = false;

Text text;
Text coin_score;

float animationElapsed3 = 0.0f;
float framesPerSecond3 = 4.0f;

string level_name;

int bulletIndex = 0;
std::vector<Bullet> bullets;
int e_bulletIndex = 0;
std::vector<Bullet> enemy_bullets;

float lastFrameTicks = 0.0f;

float animationElapsed = 0.0f;
float framesPerSecond = 5.0f;
float animationElapsed2 = 0.0f;
float framesPerSecond2 = 6.0f;

int mapWidth;
int mapHeight;
unsigned char** levelData;

float loadingAnimationTime = 0.0f;
int countDouble = 0;
int jumpCheck = 0;

SDL_Event event;
bool done = false;

//selects GAME_LEVEL functions
int state = 0;
int currentState = 0;

Text screen;
Text loading;
map<char, int> alphabet;

bool restart = false;
bool win = true;

Mix_Chunk *someSound;
Mix_Chunk *someSound2;
Mix_Chunk *someSound3;
Mix_Music *music;

vector<string> levels = { "Level With Snow 1.txt", "Level With Snow 2.txt", "Level With Snow 3.txt" };

//initializes the bullets (both player and enemy)
void initBullets() {
	for (size_t index = 0; index < MAX_BULLETS; ++index) {
		Bullet temp;
		temp.velocity.x = 0.0f;
		temp.acceleration.x = 1.0f;
		temp.active = false;
		bullets.push_back(temp);
	}

	for (int index = 0; index < 100; ++index) {
		Bullet temp;
		temp.active = false;
		enemy_bullets.push_back(temp);
	}
}

//initializes the alphabet
void initAlphabet() {
	for (int index = 0; index < 128; ++index) {
		alphabet[(char)index] = index;
	}
}

//initializes the music
void music_init() {
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	someSound = Mix_LoadWAV("jump.wav");
	someSound2 = Mix_LoadWAV("pick_up_coin.wav");
	someSound3 = Mix_LoadWAV("shoot_bullet.wav");
	music = Mix_LoadMUS("8-bit.mp3");
	//music = Mix_LoadMUS("melee.mp3");

	Mix_PlayMusic(music, -1);
}

//draws the level
void level(ShaderProgram& program) {
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0) {

				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

				float vertices[] = {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,

					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				};

				GLfloat texCoords[] = {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),

					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				};

				Matrix modelMatrix;

				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);

				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);

				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);
			}
		}
	}
}

//reads the header portion of the tile map text file
bool readHeader(ifstream& stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") {
			break;
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			mapHeight = atoi(value.c_str());
		}
	}

	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else {
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

//redas the tile set portion of the tile map text file
bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") {
			break;
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

//creates every necessary entity (player, player legs, enemies, coins)
void placeEntity(string& type, float& x, float& y) {
	if (type == "Enemy") {
		Enemy enemy;
		enemy.id = "Enemy";
		enemy.position.x = x;
		enemy.position.y = y;
		enemy.velocity.x = 0.0f;
		enemy.velocity.y = 0.0f;
		enemy.acceleration.x = 0.75f;
		enemy.acceleration.y = -1.5f;
		enemy.friction.x = 0.75f;
		enemy.size.x = TILE_SIZE;
		enemy.size.y = TILE_SIZE;
		enemy.moving = 0;
		enemy.collidedBottom = false;
		enemy.collidedTop = false;
		enemy.collidedLeft = false;
		enemy.collidedRight = false;
		enemy.air = false;
		enemy.state = 0;
		enemy.base.x = x;
		enemy.base.y = y;
		enemy.count = 0;
		enemy.active = true;
		enemy.shoot_time = 0.0f;
		enemies.push_back(enemy);
	}
	else if (type == "Player") {
		player.id = "Player";
		player.position.x = x;
		player.position.y = y;
		player.base.x = x;
		player.base.y = y;
		player.velocity.x = 0.0f;
		player.velocity.y = 0.0f;
		player.acceleration.x = 1.0f;
		player.acceleration.y = -1.5f;
		player.friction.x = 0.25f;
		player.size.x = TILE_SIZE;
		player.size.y = TILE_SIZE;
		player.collidedBottom = false;
		player.collidedTop = false;
		player.collidedLeft = false;
		player.collidedRight = false;
		player.moving = 2;
	}
	else if (type == "Coin") {
		Coin temp;
		temp.id = "Coin";
		temp.position.x = x + 0.1f;
		temp.position.y = y;
		temp.size.x = TILE_SIZE;
		temp.size.y = TILE_SIZE;
		temp.active = true;
		temp.index = 52;
		coins.push_back(temp);
	}
	else if (type == "Legs") {
		player_legs.id = "Legs";
		player_legs.position.x = x;
		player_legs.position.y = y + TILE_SIZE/2;
		player_legs.velocity.x = 0.0f;
		player_legs.velocity.y = 0.0f;
		player_legs.acceleration.x = 1.0f;
		player_legs.acceleration.y = -1.5f;
		player_legs.friction.x = 0.25f;
		player_legs.size.x = TILE_SIZE;
		player_legs.size.y = TILE_SIZE;
		player_legs.collidedBottom = false;
		player_legs.collidedTop = false;
		player_legs.collidedLeft = false;
		player_legs.collidedRight = false;
		player_legs.moving = 2;
		player_legs.index = 87;
	}
}

//reads the entity portion of the tile map text file
bool readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") {
			break;
		}
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');

			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;

			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

//reads the tile map text file
void drawLevel() {
	ifstream infile(level_name);
	string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Object Layer 1]") {
			readEntityData(infile);
		}
	}
}

//function to reset every necessary variable
//that will be reused for the next level
void resetEverything() {
	enemies.clear();
	coins.clear();

	left_ = false;
	right_ = false;
	last_moved_l = false;

	animationElapsed3 = 0.0f;
	framesPerSecond3 = 4.0f;

	bulletIndex = 0;
	bullets.clear();

	lastFrameTicks = 0.0f;

	animationElapsed = 0.0f;
	framesPerSecond = 5.0f;
	animationElapsed2 = 0.0f;
	framesPerSecond2 = 6.0f;

	loadingAnimationTime = 0.0f;
	countDouble = 0;
	jumpCheck = 0;

	viewMatrix.identity();
}

//renders the main menu
void RenderMainMenu(ShaderProgram& program, ShaderProgram& program2) {
	text.textureID = LoadTexture("pixel_font.png");

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);
	
	screen.textureID = LoadTexture("pixel_font.png");
	string Continue = "DO YOU WANT TO";
	string Continue2 = "START THE GAME?";
	string y_n = "PRESS X";

	for (int index = 0; index < Continue.length(); ++index) {
		screen.Draw(program, alphabet[Continue[index]], -0.85f + (index * 0.125f) + player.position.x + player.base.x, 0.5f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < Continue2.length(); ++index) {
		screen.Draw(program, alphabet[Continue2[index]], -0.90f + (index * 0.125) + player.position.x + player.base.x, 0.25f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < y_n.length(); ++index) {
		screen.Draw(program, alphabet[y_n[index]], -0.65f + (index * 0.175f) + player.position.x + player.base.x, -0.75f + player.position.y + player.base.y, false, true);
	}

	SDL_GL_SwapWindow(displayWindow);
}

//renders the game level
void RenderGameLevel(ShaderProgram& program, ShaderProgram& program2) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);

	//renders the snow
	snow.Render(program2);

	GLuint texture = LoadTexture("arne_sprites.png");

	//renders the level
	level(program);

	//renders enemies
	for (size_t index = 0; index < enemies.size(); ++index) {
		if (enemies[index].active) {
			enemies[index].sprite = texture;
			enemies[index].Draw(program, 80);
		}
	}

	//renders coins
	int coinCount = 0;
	for (size_t index = 0; index < coins.size(); ++index) {
		if (coins[index].active) {
			coins[index].sprite = texture;
			coins[index].Draw(program, coins[index].index);
		}
		else { ++coinCount; }
	}

	//renders active player bullets
	for (size_t i = 0; i < bullets.size(); i++) {
		if (bullets[i].active) {
			bullets[i].sprite = texture;
			bullets[i].Draw(program, 42);
		}
	}

	//renders active enemy bullets
	for (size_t i = 0; i < enemy_bullets.size(); i++) {
		if (enemy_bullets[i].active) {
			enemy_bullets[i].sprite = texture;
			enemy_bullets[i].Draw(program, 42);
		}
	}

	player.sprite = texture;
	if (last_moved_l) { player.size.x = -1 * TILE_SIZE; player_legs.size.x = -1 * TILE_SIZE; }
	else { player.size.x = TILE_SIZE; player_legs.size.x = TILE_SIZE; }
	player.Draw(program, 98);

	player_legs.sprite = texture;
	player_legs.Draw(program, player_legs.index);

	coin_score = LoadTexture("pixel_font.png");
	string coin_ = "COIN:";
	for (int index = 0; index < coin_.length(); ++index) {
		coin_score.Draw(program, alphabet[coin_[index]], 0.05f + +(index * 0.1f) + player.base.x + player.position.x, 2.025f + player.base.y + player.position.y, true);
	}

	coin_score.Draw(program, alphabet['0' + (coinCount / 10)], 0.55f + player.base.x + player.position.x, 2.025f + player.base.y + player.position.y, true);
	coin_score.Draw(program, alphabet['0' + (coinCount % 10)], 0.65f + player.base.x + player.position.x, 2.025f + player.base.y + player.position.y, true);

	//restart screen
	if (restart) {
		screen.textureID = LoadTexture("pixel_font.png");
		string Continue = "YOU DIED";
		string Continue2 = "PRESS T TO";
		string Continue3 = "TRY AGAIN";

		for (int index = 0; index < Continue.length(); ++index) {
			screen.Draw(program, alphabet[Continue[index]], -0.65f + (index * 0.125f) + player.position.x + player.base.x, 1.5f + player.position.y + player.base.y, true);
		}

		for (int index = 0; index < Continue2.length(); ++index) {
			screen.Draw(program, alphabet[Continue2[index]], -0.775f + (index * 0.125) + player.position.x + player.base.x, 1.25f + player.position.y + player.base.y, true);
		}

		for (int index = 0; index < Continue3.length(); ++index) {
			screen.Draw(program, alphabet[Continue3[index]], -0.72f + (index * 0.125f) + player.position.x + player.base.x, 1.0f + player.position.y + player.base.y, true);
		}
	}

	//win screen
	if (win) {
		screen.textureID = LoadTexture("pixel_font.png");
		string Continue = "YOU WON";
		string Continue2 = "PRESS X";
		string Continue3 = "TO CONTINUE";

		for (int index = 0; index < Continue.length(); ++index) {
			screen.Draw(program, alphabet[Continue[index]], -0.72f + (index * 0.125f) + player.position.x + player.base.x, 1.5f + player.position.y + player.base.y, true);
		}

		for (int index = 0; index < Continue2.length(); ++index) {
			screen.Draw(program, alphabet[Continue2[index]], -0.72f + (index * 0.125) + player.position.x + player.base.x, 1.25f + player.position.y + player.base.y, true);
		}

		for (int index = 0; index < Continue3.length(); ++index) {
			screen.Draw(program, alphabet[Continue3[index]], -0.95f + (index * 0.125) + player.position.x + player.base.x, 1.0f + player.position.y + player.base.y, true);
		}
	}

	SDL_GL_SwapWindow(displayWindow);
}

//renders the game over screen
void RenderGameOver(ShaderProgram& program, ShaderProgram& program2) {
	RenderGameLevel(program, program2);
}

//renders the pause screen
void RenderPauseScreen(ShaderProgram& program) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);
	screen.textureID = LoadTexture("pixel_font.png");

	string Continue = "DO YOU WANT TO";
	string Continue2 = "LEAVE THE GAME?";
	string y_n = "Y or N";

	for (int index = 0; index < Continue.length(); ++index) {
		screen.Draw(program, alphabet[Continue[index]], -1.1175f + (index * 0.125f) + player.position.x + player.base.x, 1.5f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < Continue2.length(); ++index) {
		screen.Draw(program, alphabet[Continue2[index]], -1.1725f + (index * 0.125) + player.position.x + player.base.x, 1.25f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < y_n.length(); ++index) {
		screen.Draw(program, alphabet[y_n[index]], -0.75f + (index * 0.175f) + player.position.x + player.base.x, 0.75f + player.position.y + player.base.y, false, true);
	}

	SDL_GL_SwapWindow(displayWindow);
}

//renders the loading screen
void RenderLoadingScreen(ShaderProgram& program) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	restart = false;

	string load = "Loading";
	loading.textureID = LoadTexture("pixel_font.png");
	cout << "Called" << endl;
	for (int index = 0; index < load.length(); ++index) {
		loading.Draw(program, alphabet[load[index]], 0.125f + (index * 0.13f), -0.9f, true);
	}

	SDL_GL_SwapWindow(displayWindow);
}

//renders the restart screen
void RenderRestartScreen(ShaderProgram& program) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);
	screen.textureID = LoadTexture("pixel_font.png");
	string Continue = "DO YOU WANT TO";
	string Continue2 = "RESTART THE GAME?";
	string y_n = "Y or N";

	for (int index = 0; index < Continue.length(); ++index) {
		screen.Draw(program, alphabet[Continue[index]], -1.1175f + (index * 0.125f) + player.position.x + player.base.x, 1.5f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < Continue2.length(); ++index) {
		screen.Draw(program, alphabet[Continue2[index]], -1.1725f + (index * 0.125) + player.position.x + player.base.x, 1.25f + player.position.y + player.base.y, true);
	}

	for (int index = 0; index < y_n.length(); ++index) {
		screen.Draw(program, alphabet[y_n[index]], -0.75f + (index * 0.175f) + player.position.x + player.base.x, 0.75f + player.position.y + player.base.y, false, true);
	}

	SDL_GL_SwapWindow(displayWindow);
}

//function to convert world coordinates to tile-map coordinates
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

//function to determine x-axis collision
void collisionX(Entity& test) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(test.position.x, test.position.y, gridX, gridY);

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] == 3 || levelData[y][x] == 6) {
				if (test.position.x - (fabs(test.size.x) / 2) <= (TILE_SIZE * x) + TILE_SIZE && *gridX >= x && *gridY == y) {
					test.collidedLeft = true;
					test.velocity.x = 0;
					test.position.x += fabs((test.position.x - (fabs(test.size.x) / 2)) - ((TILE_SIZE * x) + TILE_SIZE)) + 0.00000001f;
					if (test.name() == "Player" || test.name() == "Legs") {
						if (test.name() == "Player") {
							player_legs.collidedLeft = true;
							player_legs.velocity.x = 0;
							player_legs.position.x = player.position.x;
						}
						else {
							player.collidedLeft = true;
							player.velocity.x = 0;
							player.position.x = player_legs.position.x;
						}
					}

					if (test.name() == "Enemy") {
						test.size.x *= -1;
						test.acceleration.x *= -1;
					}
				}
				else if (test.position.x + (fabs(test.size.x) / 2) >= (TILE_SIZE * x) && *gridX <= x && *gridY == y) {
					test.collidedRight = true;
					test.velocity.x = 0;
					test.position.x -= fabs((test.position.x + (fabs(test.size.x) / 2)) - ((TILE_SIZE * x))) - 0.00000001f;

					if (test.name() == "Player" || test.name() == "Legs") {
						if (test.name() == "Player") {
							player_legs.collidedRight = true;
							player_legs.velocity.x = 0;
							player_legs.position.x = player.position.x;
						}
						else {
							player.collidedRight = true;
							player.velocity.x = 0;
							player.position.x = player_legs.position.x;
						}
					}

					if (test.name() == "Enemy") {
						test.size.x *= -1;
						test.acceleration.x *= -1;
					}
				}
			}
		}
	}

	//determines if the player has collided with a coin
	//if so, then the coin has been collected
	if (test.name() == "Player" || test.name() == "Legs") {
		for (size_t index = 0; index < coins.size(); ++index) {
			if (test.position.x - (fabs(test.size.x) / 2) <= (coins[index].position.x + (TILE_SIZE / 2) && fabs(test.position.y - coins[index].position.y) < TILE_SIZE) && fabs(test.position.x - coins[index].position.x) < TILE_SIZE && coins[index].active) {
				coins[index].active = false;
				Mix_PlayChannel(-1, someSound2, 0);
			}
			else if (test.position.x + (fabs(test.size.x) / 2) >= (coins[index].position.x - (TILE_SIZE / 2)) && fabs(test.position.y - coins[index].position.y) < TILE_SIZE && fabs(test.position.x - coins[index].position.x) < TILE_SIZE && coins[index].active) {
				coins[index].active = false;
				Mix_PlayChannel(-1, someSound2, 0);
			}
		}
	}

	delete gridX;
	delete gridY;
}

//function to determine y-axis collision
void collisionY(Entity& test) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(test.position.x, test.position.y, gridX, gridY);

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] == 16 || levelData[y][x] == 17 || levelData[y][x] == 18 || levelData[y][x] == 19 || levelData[y][x] == 32 || levelData[y][x] == 33 || levelData[y][x] == 34 || levelData[y][x] == 35) {
				if (test.position.y - (test.size.y / 2) <= -TILE_SIZE * y && *gridX == x && test.position.y + (test.size.y / 2) > -TILE_SIZE * y) {
					test.collidedBottom = true;
					test.moving = 0;
					test.velocity.y = 0;
					test.position.y += fabs((test.position.y - (test.size.y / 2)) - (-TILE_SIZE * y)) + 0.00000001f;
					test.air = false;
					if (test.name() == "Legs") {
						player.position.y = test.position.y + TILE_SIZE/2;
						player.collidedBottom = true;
						player.moving = 0;
						player.velocity.y = 0;
						player.air = false;
					}
				}
			}
			else if (levelData[y][x] == 6) {
				if (test.position.y - (test.size.y / 2) <= -TILE_SIZE * y && *gridX == x && test.position.y + (test.size.y / 2) > -TILE_SIZE * y) {
					test.collidedBottom = true;
					test.moving = 0;
					test.velocity.y = 0;
					test.position.y += fabs((test.position.y - (test.size.y / 2)) - (-TILE_SIZE * y)) + 0.00000001f;
					test.air = false;
					if (test.name() == "Legs") {
						player.position.y = test.position.y + TILE_SIZE / 2;
						player.collidedBottom = true;
						player.moving = 0;
						player.velocity.y = 0;
						player.air = false;
					}
				}
				else if (test.position.y + (test.size.y / 2) >= (-TILE_SIZE * y) - TILE_SIZE && *gridX == x && test.position.y - (test.size.y / 2) < (-TILE_SIZE * y) - TILE_SIZE) {
					test.collidedTop = true;
					test.velocity.y = 0;
					test.position.y -= fabs((test.position.y + (test.size.y / 2)) - ((-TILE_SIZE * y) - TILE_SIZE)) - 0.00000001f;
					if (test.name() == "Player") {
						player_legs.collidedTop = true;
						player_legs.velocity.y = 0;
						player_legs.position.y = test.position.y - TILE_SIZE / 2;
					}
				}
			}
			else {
				test.collidedBottom = false;
				test.collidedTop = false;
			}
		}
	}

	delete gridX;
	delete gridY;
}

//function to determine whether the enemy can jump to an above platform
void jumpAbility(Enemy& enemy) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(enemy.position.x, enemy.position.y, gridX, gridY);

	if (!enemy.air) {
		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			for (int x = 0; x < LEVEL_WIDTH; x++) {
				if (levelData[y][x] == 6) {
					if (sqrtf(powf(((enemy.position.y / -TILE_SIZE) - (y)), 2) + powf(((enemy.position.x / TILE_SIZE) - (x)), 2)) <= 4.0f && *gridY > y && enemy.moving < 1 && enemy.velocity.y == 0 && ((*gridX < x && enemy.acceleration.x > 0) || (*gridX > x && enemy.acceleration.x < 0))) {
						enemy.velocity.y += 1.0f;
						enemy.moving += 1;
						enemy.air = true;
						break;
					}
				}
			}
		}
	}

	delete gridX;
	delete gridY;
}

//function to determine whether the enemy can fall down
void fallAbility(Enemy& enemy) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(enemy.position.x, enemy.position.y, gridX, gridY);
	
	if (!enemy.air) {
		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			for (int x = 0; x < LEVEL_WIDTH; x++) {
				int temp = (*gridY + 6);
				if (temp > LEVEL_HEIGHT - 2) { temp = LEVEL_HEIGHT - 2; }
				if (levelData[*gridY + 1][*gridX + 1] == 0 && (levelData[temp][*gridX + 2] != 16 && levelData[temp][*gridX + 2] != 17 && levelData[temp][*gridX + 2] != 18 && levelData[temp][*gridX + 2] != 19 && levelData[temp][*gridX + 2] != 32 && levelData[temp][*gridX + 2] != 33 && levelData[temp][*gridX + 2] != 34 && levelData[temp][*gridX + 2] != 35 && levelData[temp][*gridX + 2] != 6)) {
					if ((float)(enemy.position.x / TILE_SIZE) + (enemy.size.x / 2) >= (x + 1) - (TILE_SIZE / 2) && enemy.acceleration.x > 0) {
						enemy.acceleration.x *= -1;
						enemy.size.x *= -1;
					}
				}
				else if (levelData[*gridY + 1][*gridX - 1] == 0 && (levelData[temp][*gridX - 2] != 16 && levelData[temp][*gridX - 2] != 17 && levelData[temp][*gridX - 2] != 18 && levelData[temp][*gridX - 2] != 19 && levelData[temp][*gridX - 2] != 32 && levelData[temp][*gridX - 2] != 33 && levelData[temp][*gridX - 2] != 34 && levelData[temp][*gridX - 2] != 35 && levelData[temp][*gridX - 2] != 6)) {
					if ((float)(enemy.position.x / TILE_SIZE) - (enemy.size.x / 2) <= (x - 1) + (TILE_SIZE / 2) && enemy.acceleration.x < 0) {
						enemy.acceleration.x *= -1;
						enemy.size.x *= -1;
					}
				}
			}
		}
	}

	delete gridX;
	delete gridY;
}

//enemy state where plaer is not in range
//so the enemy just moves around the map
//and jumps to an above platform if it is possible
void enemyUpdateNormal(Enemy& enemy, float elapsed) {
	//if player in range, enemyState = 1
	float distx = player.position.x - enemy.position.x;
	if (sqrtf(powf((distx), 2) + powf((player.position.y - enemy.position.y), 2)) <= 0.5f) {
		enemy.state = 1;
		enemy.count = 0;
	}
	if (enemy.count <= 0) {
		jumpAbility(enemy);
		fallAbility(enemy);
		enemy.position.x += elapsed / 2.0 * enemy.acceleration.x;

		collisionX(enemy);

		enemy.velocity.y += enemy.acceleration.y * elapsed;
		enemy.position.y += enemy.velocity.y*elapsed;

		collisionY(enemy);
	}
	else { enemy.count -= 1; }
}

//enemy state where player within the enemy's range
//so the enemy statrs shooting at the player
void enemyUpdateAttacking(Enemy& enemy, float elapsed) {
	if (player.position.x < enemy.position.x) {
		if (enemy.acceleration.x > 0) { enemy.acceleration.x *= -1; }
		if (enemy.size.x > 0) { enemy.size.x *= -1; }
	}
	else if (player.position.x > enemy.position.x) {
		if (enemy.acceleration.x < 0) { enemy.acceleration.x *= -1; }
		if (enemy.size.x < 0) { enemy.size.x *= -1; }
	}

	enemy.shoot_time += elapsed;
	if (enemy.shoot_time >= 1.5f) {
		enemy.shoot_time = 0.0f;

		enemy_bullets[e_bulletIndex].active = true;
		if (enemy.size.x < 0) { enemy_bullets[e_bulletIndex].direction = -1; }
		else { enemy_bullets[e_bulletIndex].direction = 1; }
		enemy_bullets[e_bulletIndex].position.x = enemy.position.x;
		enemy_bullets[e_bulletIndex].position.y = enemy.position.y;
		if (e_bulletIndex + 1 == 100) {
			e_bulletIndex = 0;
		}

		else {
			++e_bulletIndex;
		}
	}

	enemy.velocity.y += enemy.acceleration.y * elapsed;
	enemy.position.y += enemy.velocity.y * elapsed;

	collisionY(enemy);

	//if player not in range, enemyState = 2
	float distx = player.position.x - enemy.position.x;
	if (sqrtf(powf((distx), 2) + powf((player.position.y - enemy.position.y), 2)) > 0.75f) {
		enemy.state = 2;
	}
}

//enemy state where player is not in range
//and after the enemy returns to its original position
//it waits a few seconds becoming moving again
void enemyUpdateReturning(Enemy& enemy, float elapsed) {
	float distx = player.position.x - enemy.position.x;
	if (sqrtf(powf((distx), 2) + powf((player.position.y - enemy.position.y), 2)) <= 0.75f) {
		enemy.state = 1;
		return;
	}

	if (fabs(enemy.position.x - enemy.base.x) <= 0.05f) {
		enemy.state = 0;
		if (enemy.acceleration.x < 0) { enemy.acceleration.x *= -1; }
		if (enemy.size.x < 0) { enemy.size.x *= -1; }
		enemy.position.x = enemy.base.x;
		enemy.count = 40;
	}
	else {
		if (enemy.count >= 4) {
			if (enemy.position.x < enemy.base.x) {
				if (enemy.acceleration.x < 0) { enemy.acceleration.x *= -1; }
				if (enemy.size.x < 0) { enemy.size.x *= -1; }
			}
			else if (enemy.position.x > enemy.base.x) {
				if (enemy.acceleration.x > 0) { enemy.acceleration.x *= -1; }
				if (enemy.size.x > 0) { enemy.size.x *= -1; }
			}
			enemy.position.x += elapsed / 2.0f * enemy.acceleration.x;
			collisionX(enemy);
		}
		enemy.count += 1;
		enemy.velocity.y += enemy.acceleration.y * elapsed;
		enemy.position.y += enemy.velocity.y*elapsed;

		collisionY(enemy);
	}
}

//enemy AI states (idle, attacking, returning)
enum EnemyState { IDLE, CHASING, RETURNING };

void enemyUpdate(Enemy& enemy, float elapsed) {
	switch (enemy.state) {
	case IDLE:
		enemyUpdateNormal(enemy, elapsed);
		break;
	case CHASING:
		enemyUpdateAttacking(enemy, elapsed);
		break;
	case RETURNING:
		enemyUpdateReturning(enemy, elapsed);
		break;
	}
}

//enemy collision detection with player bullets
void bulletTest(Enemy& enemy) {
	for (size_t index = 0; index < bullets.size(); ++index) {
		if (bullets[index].active && enemy.active) {
			if (enemy.position.x >= player.position.x) {
				if (enemy.position.x - (fabs(enemy.size.x) / 2) <= bullets[index].position.x + (bullets[index].size.x / 2) && enemy.position.y + (fabs(enemy.size.y) / 2) >= bullets[index].position.y - (bullets[index].size.y / 2) && enemy.position.y - (enemy.size.y / 2) <= bullets[index].position.y + (bullets[index].size.y / 2)) {
					enemy.active = false;
					bullets[index].active = false;
				}
			}
			else {
				if (enemy.position.x + (fabs(enemy.size.x) / 2) >= bullets[index].position.x - (bullets[index].size.x / 2) && enemy.position.y + (fabs(enemy.size.y) / 2) >= bullets[index].position.y - (bullets[index].size.y / 2) && enemy.position.y - (enemy.size.y / 2) <= bullets[index].position.y + (bullets[index].size.y / 2)) {
					enemy.active = false;
					bullets[index].active = false;
				}
			}
		}
	}
}

//player collision detection with enemy bullets
void p_bulletTest() {
	for (size_t index = 0; index < enemy_bullets.size(); ++index) {
		if (enemy_bullets[index].active) {
			if (player.position.x >= enemy_bullets[index].position.x) {
				if ((player.position.x - (fabs(player.size.x) / 2) <= enemy_bullets[index].position.x + (enemy_bullets[index].size.x / 2) &&
					player.position.y + (fabs(player.size.y) / 2) >= enemy_bullets[index].position.y - (enemy_bullets[index].size.y / 2) &&
					player.position.y - (player.size.y / 2) <= enemy_bullets[index].position.y + (bullets[index].size.y / 2)) || 
					(player_legs.position.x - (fabs(player_legs.size.x) / 2) <= enemy_bullets[index].position.x + (enemy_bullets[index].size.x / 2) &&
					player_legs.position.y + (fabs(player_legs.size.y) / 2) >= enemy_bullets[index].position.y - (enemy_bullets[index].size.y / 2) &&
					player_legs.position.y - (player_legs.size.y / 2) <= enemy_bullets[index].position.y + (bullets[index].size.y / 2))) {

					//player is dead - game over
					restart = true;
					state = 2;
					currentState = 2;
				}
			}
			else {
				if ((player.position.x + (fabs(player.size.x) / 2) >= enemy_bullets[index].position.x - (enemy_bullets[index].size.x / 2) &&
					player.position.y + (fabs(player.size.y) / 2) >= enemy_bullets[index].position.y - (enemy_bullets[index].size.y / 2) &&
					player.position.y - (player.size.y / 2) <= enemy_bullets[index].position.y + (enemy_bullets[index].size.y / 2)) || 
					(player_legs.position.x + (fabs(player_legs.size.x) / 2) >= enemy_bullets[index].position.x - (enemy_bullets[index].size.x / 2) &&
					player_legs.position.y + (fabs(player_legs.size.y) / 2) >= enemy_bullets[index].position.y - (enemy_bullets[index].size.y / 2) &&
					player_legs.position.y - (player_legs.size.y / 2) <= enemy_bullets[index].position.y + (enemy_bullets[index].size.y / 2))) {

					//player is dead - game over
					restart = true;
					state = 2;
					currentState = 2;
				}
			}
		}
	}
}


void UpdateMainMenu(float& elapsed) {
	initAlphabet();
}

void UpdateGameLevel(float elapsed) {
	//calculates whether the player won or not
	int next_level = 0;
	for (int index = 0; index < coins.size(); ++index) {
		if (!coins[index].active) { ++next_level; }
	}
	if (next_level == coins.size()) { state = 6; win = true; }

	//calculations for the enemy bullets
	for (int index = 0; index < 100; ++index) {
		if (enemy_bullets[index].active) {
			enemy_bullets[index].position.x += elapsed*enemy_bullets[index].direction;
			enemy_bullets[index].lifetime += elapsed;
		}
		if (enemy_bullets[index].lifetime >= 1.25f) {
			enemy_bullets[index].lifetime = 0.0f;
			enemy_bullets[index].active = false;
		}
	}

	//animates the coins
	animationElapsed += elapsed;
	if (animationElapsed > 1.0 / framesPerSecond) {
		animationElapsed = 0.0f;
		for (int index = 0; index < coins.size(); ++index) {
			if (coins[index].index == 54) {
				coins[index].index = 51;
			}
			else { ++coins[index].index; }
		}
	}

	//animates the movement of the legs
	//and calculates the movement of the player
	animationElapsed2 += elapsed;
	if (player_legs.moving != 0) { animationElapsed3 += elapsed; }
	else { animationElapsed3 = 0.0f; }
	if ((left_ || right_) && player_legs.index != 71 && player.moving == 0) {
		if (animationElapsed2 > 1.0 / framesPerSecond2) {
			animationElapsed2 = 0.0f;
			if (player_legs.index == 87) { player_legs.index = 63; }
			if (player_legs.index == 70) { player_legs.index = 63; }
			++player_legs.index;
		}
	}
	else if ((left_ || right_) && player_legs.index == 71 && player.moving == 0) {
		if (animationElapsed2 > 1.0 / framesPerSecond2) {
			animationElapsed2 = 0.0f;
			player_legs.index = 66;
		}
	}
	else if (player.moving != 0 && (player_legs.index == 64 || player_legs.index == 87)) {
		if (animationElapsed3 > 1.0 / framesPerSecond3) {
			animationElapsed3 = 0.0f;
			if (player_legs.index == 87) { player_legs.index = 64; }
			else { player_legs.index = 71; }
		}
	}
	else if (player.moving != 0 && player_legs.index == 71) {}
	else { player_legs.index = 87; }

	if (left_) { 
		player.velocity.x += elapsed * -player.acceleration.x; 
		player.position.x += elapsed * player.velocity.x;

		player_legs.velocity.x += elapsed * -player_legs.acceleration.x;
		player_legs.position.x += elapsed * player_legs.velocity.x;
	}
	else if (right_) {
		player.velocity.x += elapsed * player.acceleration.x;
		player.position.x += elapsed * player.velocity.x;

		player_legs.velocity.x += elapsed * player_legs.acceleration.x;
		player_legs.position.x += elapsed * player_legs.velocity.x;
	}
	else {
		player.velocity.x = lerp(player.velocity.x, 0.0f, elapsed * player.friction.x);
		player.position.x += elapsed * player.velocity.x;

		player_legs.velocity.x = lerp(player_legs.velocity.x, 0.0f, elapsed * player_legs.friction.x);
		player_legs.position.x += elapsed * player_legs.velocity.x;
	}

	collisionX(player);
	collisionX(player_legs);

	player.velocity.y += player.acceleration.y * elapsed;
	player.position.y += player.velocity.y*elapsed;

	player_legs.velocity.y += player_legs.acceleration.y * elapsed;
	player_legs.position.y += player_legs.velocity.y*elapsed;

	collisionY(player_legs);
	collisionY(player);

	//player bullet calculations
	for (int index = 0; index < MAX_BULLETS; ++index) {
		if (bullets[index].active) {
			bullets[index].position.x += elapsed*bullets[index].direction;
			bullets[index].lifetime += elapsed;
		}
		else {
			bullets[index].position.x = player.position.x;
			bullets[bulletIndex].position.y = player.position.y - (TILE_SIZE / 4.0f);
		}
		if (bullets[index].lifetime >= 1.25f) {
			bullets[index].lifetime = 0.0f;
			bullets[index].position.x = player.position.x;
			bullets[bulletIndex].position.y = player.position.y - (TILE_SIZE / 4.0f);
			bullets[index].active = false;
		}
	}

	if (!restart && !win) { p_bulletTest(); }

	for (size_t index = 0; index < enemies.size(); ++index) {
		if (enemies[index].active) {
			enemyUpdate(enemies[index], elapsed);
			bulletTest(enemies[index]);
		}
	}

	snow.Update(elapsed);

	viewMatrix.identity();
	viewMatrix.Translate(-player.position.x, -player.position.y, 0.0f);
}

//index to choose the levels
int level_num = -1;

//update loading screen state
void UpdateLoadingScreen(float elapsed) {
	if (loadingAnimationTime >= 2.5f) {
		state = 1;
		currentState = 1;
		//after the third level, it loops back to the first level
		if (!restart && win) { 
			if (level_num + 1 == levels.size()) { level_num = -1; }
			level_name = levels[++level_num]; }
		if (win) { win = false; }
		drawLevel();
		initBullets();
		restart = false;
	}
	else { loadingAnimationTime += elapsed; }
}

//update game over state
void UpdateGameOver(float elapsed) {
	UpdateGameLevel(elapsed);
}

//main menu state
void ProcessMainMenuInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_X) {
			state = 4;
			currentState = 4;
		}
	}
}

//main game level state
void ProcessGameLevelInput() {
	//player movement
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_A]) {
		left_ = true;
		right_ = false;
		last_moved_l = true;
	}
	else if (keys[SDL_SCANCODE_D]) {
		left_ = false;
		right_ = true;
		last_moved_l = false;
	}
	else {
		left_ = false;
		right_ = false;
	}
	
	while (SDL_PollEvent(&event)) {
		//player wants to pause/leave the game
		if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			state = 3;
			currentState = 3;
		}
		//player wants to restart
		else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
			state = 5;
			currentState = 5;
		}
		//player presses W to jump (max of two jumps)
		else if (event.key.keysym.scancode == SDL_SCANCODE_W && player.moving <= 1) {
			jumpCheck += 1;
			if (jumpCheck % 3 == 1) {
				player.moving += 1;
				player_legs.moving += 1;
				player.velocity.y += 0.75f;
				player_legs.velocity.y += 0.75f;
				if (player_legs.moving == 1) { player_legs.index = 64; }
				jumpCheck += 1;
				Mix_PlayChannel(-1, someSound, 0);
			}
			else { jumpCheck = 0; }
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			countDouble += 1;
			if (countDouble % 2 == 1) {
				bullets[bulletIndex].active = true;
				if (last_moved_l) { bullets[bulletIndex].direction = -1; }
				else { bullets[bulletIndex].direction = 1; }
				if (bulletIndex + 1 == MAX_BULLETS) {
					bulletIndex = 0;
				}

				else {
					++bulletIndex;
				}

				Mix_PlayChannel(-1, someSound3, 0);
			}
		}
		else { countDouble = 0; }
	}
}

//state where user dies
void ProcessGameOverInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_T) {
			resetEverything();
			state = 4;
			currentState = 4;
		}
	}
}

//state where user decides wheter they want to leave the game or not
//also acts as a pause screen
void ProcessPauseScreen() {
	while (SDL_PollEvent(&event)) {
		if (event.key.keysym.scancode == SDL_SCANCODE_Y) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_N) {
			state = 1;
			currentState = 1;
		}
	}
}

//state where user presses R to restart
void ProcessRestartScreen() {
	while (SDL_PollEvent(&event)) {
		if (event.key.keysym.scancode == SDL_SCANCODE_Y) {
			resetEverything();
			state = 4;
			currentState = 4;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_N) {
			state = 1;
			currentState = 1;
		}
	}
}

//win state
void ProcessWin() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_X) {
			resetEverything();
			state = 4;
			currentState = 4;
		}
	}
}

//various states for the game
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_PAUSE_SCREEN, STATE_LOADING_SCREEN, STATE_RESTART_SCREEN, STATE_WIN};

void Render(ShaderProgram& program, ShaderProgram& program2) {
	switch (state) {
	case STATE_MAIN_MENU:
		RenderMainMenu(program, program2);
		break;
	case STATE_GAME_LEVEL:
		RenderGameLevel(program, program2);
		break;
	case STATE_GAME_OVER:
		RenderGameLevel(program, program2);
		break;
	case STATE_PAUSE_SCREEN:
		RenderPauseScreen(program);
		break;
	case STATE_LOADING_SCREEN:
		RenderLoadingScreen(program);
		break;
	case STATE_RESTART_SCREEN:
		RenderRestartScreen(program);
		break;
	case STATE_WIN:
		RenderGameLevel(program, program2);
		break;
	}
}
void Update(float elapsed) {
	switch (state) {
	case STATE_MAIN_MENU:
		UpdateMainMenu(elapsed);
		break;
	case STATE_GAME_LEVEL:
		UpdateGameLevel(elapsed);
		break;
	case STATE_LOADING_SCREEN:
		UpdateLoadingScreen(elapsed);
		break;
	case STATE_GAME_OVER:
		UpdateGameLevel(elapsed);
		break;
	case STATE_RESTART_SCREEN:
		UpdateGameLevel(elapsed);
		break;
	case STATE_WIN:
		UpdateGameLevel(elapsed);
		break;
	}
}
void ProcessInput() {
	switch (state) {
	case STATE_MAIN_MENU:
		ProcessMainMenuInput();
		break;
	case STATE_GAME_LEVEL:
		ProcessGameLevelInput();
		break;
	case STATE_GAME_OVER:
		ProcessGameOverInput();
		break;
	case STATE_PAUSE_SCREEN:
		ProcessPauseScreen();
		break;
	case STATE_RESTART_SCREEN:
		ProcessRestartScreen();
		break;
	case STATE_WIN:
		ProcessWin();
		break;
	}
}

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 800, 800);
}

int main(int argc, char *argv[])
{
	Setup();
	ShaderProgram program("vertex_textured.glsl", "fragment_textured.glsl");
	ShaderProgram program2("vertex.glsl", "fragment.glsl");
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//initialize music
	music_init();

	while (!done) {
		ProcessInput();

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed;
		if (fixedElapsed > (1.0f / 60.0f) * 6) {
			fixedElapsed = (1.0f / 60.0f) * 6;
		}
		while (fixedElapsed >= (1.0f / 60.0f)) {
			fixedElapsed -= (1.0f / 60.0f);
			Update(1.0f / 60.0f);
		}
		Update(fixedElapsed);

		Render(program, program2);
	}

	SDL_Quit();
	return 0;
}