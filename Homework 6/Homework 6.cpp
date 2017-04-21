/*
Name: Matthew Persad
NetID: mp3685
Homework #6

In this project, you are playing a basic demo. You use "A" to move left, "D" to move right,
and the return key is to jump. You can jump infinitely. Your goal is to collect the coins.

This is Homework #4, but with sounds for jumping and collecting
coins, and there is music playing in the background.
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

using namespace std;

#define TILE_SIZE 0.1f
#define LEVEL_HEIGHT 16
#define LEVEL_WIDTH 128
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

Matrix projectionMatrix;
Matrix viewMatrix;

Mix_Chunk *someSound;
Mix_Chunk *someSound2;
Mix_Music *music;

void music_init() {
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	someSound = Mix_LoadWAV("jump.wav");
	someSound2 = Mix_LoadWAV("pick_up_coin.wav");
	music = Mix_LoadMUS("music.mp3");

	Mix_PlayMusic(music, -1);
}

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

class Text {
public:
	Text() {}
	Text(unsigned int texture) {
		textureID = texture;
	}

	void Draw(ShaderProgram& program, int character, float x, float y, bool test = false) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		float u = (float)(((int)character) % 16) / (float)16;
		float v = (float)(((int)character) / 16) / (float)16;
		float width = 1.0 / (float)16;
		float height = 1.0 / (float)16;
		GLfloat texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height };
		float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

		modelMatrix.identity();
		if (test) { modelMatrix.Scale(0.125f, 0.125f, 1.0f); }
		else { modelMatrix.Scale(0.25f, 0.25f, 1.0f); }
		modelMatrix.Translate(x, y, 0.0f);

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
};

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

class Entity {
public:
	Entity() {}
	Entity(SheetSprite& sprite_) : sprite(sprite_) {}

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
	Vector3 size;
	SheetSprite sprite;
	Matrix modelMatrix;
	bool collected;
};

float lastFrameTicks = 0.0f;

SDL_Event event;
bool done = false;

//selects GAME_LEVEL functions
int state = 1;

int mapWidth;
int mapHeight;
unsigned char** levelData;
unsigned char level1Data[LEVEL_HEIGHT][LEVEL_WIDTH];

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
	else { // allocate our map data     
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

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
						// be careful, the tiles in this format are indexed from 1 not 0                     
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

Entity player;
vector<Entity> enemies;
vector<Entity> coins;

void placeEntity(string& type, float& x, float& y) {
	if (type == "Enemy") {
		Entity enemy;
		enemy.position.x = x;
		enemy.position.y = y;
		enemy.velocity.y = 0.0f;
		enemy.acceleration.y = -1.5f;
		enemy.size.x = TILE_SIZE;
		enemy.size.y = TILE_SIZE;
		enemies.push_back(enemy);
	}
	else if (type == "Player") {
		player.position.x = x;
		player.position.y = y;
		player.acceleration.y = -1.5f;
		player.size.x = TILE_SIZE;
		player.size.y = TILE_SIZE;
	}
	else if (type == "Entity") {
		Entity temp;
		temp.position.x = x + 0.1f;
		temp.position.y = y;
		temp.size.x = TILE_SIZE;
		temp.size.y = TILE_SIZE;
		temp.collected = false;
		coins.push_back(temp);
	}
}

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

void drawLevel() {
	ifstream infile("Homework 4.txt");
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

bool left_ = false;
bool right_ = false;
bool last_moved_l = false;

void RenderMainMenu(ShaderProgram& program) {
	//nothing
}

void RenderGameLevel(ShaderProgram& program) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);

	GLuint texture = LoadTexture("arne_sprites.png");

	int inner_count = 0;
	level(program);

	for (size_t index = 0; index < enemies.size(); ++index) {
		enemies[index].sprite = texture;
		enemies[index].Draw(program, 80);
	}

	for (size_t index = 0; index < coins.size(); ++index) {
		if (!coins[index].collected) {
			coins[index].sprite = texture;
			coins[index].Draw(program, 52);
		}
	}

	player.sprite = texture;
	if (last_moved_l) { player.size.x = -1 * TILE_SIZE; }
	else { player.size.x = TILE_SIZE; }
	player.Draw(program, 98);

	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameOver(ShaderProgram& program) {
	//nothing
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

void collisionX(Entity& test) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(test.position.x, test.position.y, gridX, gridY);

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] == 3 || levelData[y][x] == 6) {
				if (test.position.x - (fabs(test.size.x) / 2) <= (TILE_SIZE * x) + TILE_SIZE && *gridX >= x && *gridY == y) {
					test.velocity.x = 0;
					test.position.x += fabs((test.position.x - (fabs(test.size.x) / 2)) - ((TILE_SIZE * x) + TILE_SIZE)) + 0.00000001f;
				}
				else if (test.position.x + (fabs(test.size.x) / 2) >= (TILE_SIZE * x) && *gridX <= x && *gridY == y) {
					test.velocity.x = 0;
					test.position.x -= fabs((test.position.x + (fabs(test.size.x) / 2)) - ((TILE_SIZE * x))) + 0.00000001f;
				}
			}
		}
	}

	for (size_t index = 0; index < coins.size(); ++index) {
		if (test.position.x - (fabs(test.size.x) / 2) <= (coins[index].position.x + (TILE_SIZE / 2) && fabs(test.position.y - coins[index].position.y) < TILE_SIZE) && fabs(test.position.x - coins[index].position.x) < TILE_SIZE && !coins[index].collected) {
			coins[index].collected = true;
			Mix_PlayChannel(-1, someSound2, 0);
		}
		else if (test.position.x + (fabs(test.size.x) / 2) >= (coins[index].position.x - (TILE_SIZE / 2)) && fabs(test.position.y - coins[index].position.y) < TILE_SIZE && fabs(test.position.x - coins[index].position.x) < TILE_SIZE && !coins[index].collected) {
			coins[index].collected = true;
			Mix_PlayChannel(-1, someSound2, 0);
		}
	}
}

void collisionY(Entity& test) {
	int* gridX = new int(0);
	int* gridY = new int(0);
	worldToTileCoordinates(test.position.x, test.position.y, gridX, gridY);

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] == 16 || levelData[y][x] == 17 || levelData[y][x] == 18 || levelData[y][x] == 19 || levelData[y][x] == 32 || levelData[y][x] == 33 || levelData[y][x] == 34 || levelData[y][x] == 35) {
				if (test.position.y - (test.size.y / 2) <= -TILE_SIZE * y && *gridX == x && test.position.y + (test.size.y / 2) > -TILE_SIZE * y) {
					test.velocity.y = 0;
					test.position.y += fabs((test.position.y - (test.size.y / 2)) - (-TILE_SIZE * y)) + 0.00000001f;
				}
			}
			else if (levelData[y][x] == 6) {
				if (test.position.y - (test.size.y / 2) <= -TILE_SIZE * y && *gridX == x && test.position.y + (test.size.y / 2) > -TILE_SIZE * y) {
					test.velocity.y = 0;
					test.position.y += fabs((test.position.y - (test.size.y / 2)) - (-TILE_SIZE * y)) + 0.00000001f;
				}
				else if (test.position.y + (test.size.y / 2) >= (-TILE_SIZE * y) - TILE_SIZE && *gridX == x && test.position.y - (test.size.y / 2) < (-TILE_SIZE * y) - TILE_SIZE) {
					test.velocity.y = 0;
					test.position.y -= fabs((test.position.y + (test.size.y / 2)) - ((-TILE_SIZE * y) - TILE_SIZE)) - 0.00000001f;
				}
			}
		}
	}
}

void UpdateMainMenu(float& elapsed) {
	//nothing
}

void UpdateGameLevel(float elapsed) {
	if (left_) { player.position.x -= elapsed; }
	else if (right_) { player.position.x += elapsed; }

	player.velocity.y += player.acceleration.y * elapsed;
	player.position.y += player.velocity.y*elapsed;

	collisionX(player);
	collisionY(player);

	for (size_t index = 0; index < enemies.size(); ++index) {
		enemies[index].velocity.y += enemies[index].acceleration.y * elapsed;
		enemies[index].position.y += enemies[index].velocity.y*elapsed;
		collisionX(enemies[index]);
		collisionY(enemies[index]);
	}

	viewMatrix.identity();
	viewMatrix.Translate(-player.position.x, -player.position.y, 0.0f);
}

void ProcessMainMenuInput() {
	//nothing
}

void ProcessGameLevelInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			player.velocity.y = 0.5f;
			Mix_PlayChannel(-1, someSound, 0);
		}
	}

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
}

void ProcessGameOverInput() {
	//nothing
}

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

void Render(ShaderProgram& program) {
	switch (state) {
	case STATE_MAIN_MENU:
		RenderMainMenu(program);
		break;
	case STATE_GAME_LEVEL:
		RenderGameLevel(program);
		break;
	case STATE_GAME_OVER:
		RenderGameOver(program);
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
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	music_init();

	drawLevel();

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

		Render(program);
	}

	Mix_FreeChunk(someSound);  
	Mix_FreeChunk(someSound2);
	Mix_FreeMusic(music);

	SDL_Quit();
	return 0;
}