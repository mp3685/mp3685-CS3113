/*
Name: Matthew Persad
NetID: mp3685
Homework #3

In this project, you can play one level of space invaders. 
You shoot by pressing the return key, and you move by pressing 'A' for left, and 'D' for right.
If the aliens get too close to the player, it is an instant game over, and the screen prompts the words "Space Invaders".
Instead, if the player kills all of the aliens, then the player wins, and the screen prompts the words "Space Invaders".
*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

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
	Matrix projectionMatrix;
	Matrix viewMatrix;
};

class SheetSprite { 
public:      
	SheetSprite() {}
	SheetSprite(unsigned int textureID_) : textureID(textureID_) {}

	void Draw(ShaderProgram& program, int character) {
		glBindTexture(GL_TEXTURE_2D, textureID); 

		float u = (float)(((int)character) % 2) / (float)2;
		float v = (float)(((int)character) / 2) / (float)4;
		float width = 1.0 / (float)2;
		float height = 1.0 / (float)4;

		GLfloat texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height }; 
		
		float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

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
	Matrix projectionMatrix;
	Matrix viewMatrix;
};


class Vector3 {
public:
	Vector3() {}
	Vector3(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	
	float x;   
	float y;    
	float z; 
};

class Entity {
public:
	Entity() {}
	Entity(SheetSprite& sprite_) : sprite(sprite_) {}

	void Draw(ShaderProgram& program, int index) {
		sprite.Draw(program, index);
	}

	void Draw(ShaderProgram& program) {
		modelMatrix.identity();
		modelMatrix.Translate(position.x, position.y, 0.0f);
		modelMatrix.Scale(size.x, size.y, 1.0f);

		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}
	
	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	SheetSprite sprite; 
	Matrix modelMatrix;
	Matrix projectionMatrix;
	Matrix viewMatrix;
	bool shot = false;
};

float lastFrameTicks = 0.0f;

Text text;

std::vector<Entity> enemies1;
std::vector<Entity> enemies2;
std::vector<Entity> enemies3;
std::vector<Entity> enemies4;
std::vector<Entity> enemies5;
std::vector<Entity> enemies6;
std::vector<Entity> enemies7;
std::vector<std::vector<Entity>> enemies = { enemies1, enemies2, enemies3, enemies4, enemies5, enemies6, enemies7 };

#define MAX_BULLETS 30 
int bulletIndex = 0;  
std::vector<Entity> bullets;

void initEnemies() {
	float x = -0.8f;
	float y = 0.9f;

	for (size_t index = 0; index < enemies.size(); ++index) {
		for (size_t index2 = 0; index2 < 6; ++index2) {
			SheetSprite sprite(LoadTexture("space_invaders.png"));
			Entity enemy(sprite);
			enemies[index].push_back(enemy);
			enemies[index][index2].position.x = x;
			enemies[index][index2].position.y = y;
			enemies[index][index2].size.x = 0.125f;
			enemies[index][index2].size.y = 0.25f;
			x += 0.3f;
		}
		x = -0.8f;
		y -= 0.175f;
	}

	for (int i = 0; i < MAX_BULLETS; i++) {
		Entity bullet;
		bullet.position.y = -2000.0f;
		bullet.shot = false;
		bullet.size.x = 0.025f;
		bullet.size.y = 0.075f;
		bullets.push_back(bullet);
	}
}

Entity player;
bool left, right;
bool shoot = false;
bool temp_ = false;

SDL_Event event;
bool done = false;

int state = 0;

void shootBullet() {
	bulletIndex++;
	if (bulletIndex > MAX_BULLETS - 1) { bulletIndex = 0; }
}

void RenderMainMenu(ShaderProgram& program) {
	text.textureID = LoadTexture("pixel_font.png");

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);

	std::vector<int> space_invaders = { 83, 80, 65, 67, 69, 73, 78, 86, 65, 68, 69, 82, 83 };
	float x = -1.85f;
	float y = 1.0f;

	for (size_t index = 0; index < space_invaders.size(); ++index) {
		if (index == 5) { x = -3.9f-(0.85f*4); y -= 1.0f; }
		text.Draw(program, space_invaders[index], x+(index*0.85f), y);
	}

	std::vector<int> start_z = { 80, 82, 69, 83, 83, 32, 90, 32, 84, 79, 32, 83, 84, 65, 82, 84 };

	for (size_t index = 0; index < start_z.size(); ++index) {
		text.Draw(program, start_z[index], -6.4f + (index*0.85f), -6.0f, true);
	}

	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameLevel(ShaderProgram& program, ShaderProgram& program2) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0.0, 0.0, 0.0f);

	for (size_t index = 0; index < enemies.size(); ++index) {
		for (size_t index2 = 0; index2 < enemies[index].size(); ++index2) {
			if (!enemies[index][index2].shot) {
				enemies[index][index2].sprite.modelMatrix.identity();
				enemies[index][index2].sprite.modelMatrix.Translate(enemies[index][index2].position.x, enemies[index][index2].position.y, 0.0f);
				enemies[index][index2].sprite.modelMatrix.Scale(0.125f, 0.25f, 1.0f);
				enemies[index][index2].Draw(program, index);
			}
		}
	}

	SheetSprite sprite(LoadTexture("space_invaders.png"));
	player.sprite = sprite;

	player.sprite.modelMatrix.identity();
	player.sprite.modelMatrix.Translate(player.position.x, player.position.y, 0.0f);
	player.sprite.modelMatrix.Scale(0.25f, 0.25f, 1.0f);
	player.Draw(program, 7);

	for (size_t i = 0; i < bullets.size(); i++) { 
		if (bullets[i].shot) {
			bullets[i].Draw(program2);
		}
	}

	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameOver(ShaderProgram& program) {
	text.textureID = LoadTexture("pixel_font.png");

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0f);

	std::vector<int> play_again = { 83, 80, 65, 67, 69, 73, 78, 86, 65, 68, 69, 82, 83 };
	float x = -1.85f;
	float y = 1.0f;

	for (size_t index = 0; index < play_again.size(); ++index) {
		if (index == 5) { x = -3.9f - (0.85f * 4); y -= 1.0f; }
		text.Draw(program, play_again[index], x + (index*0.85f), y);
	}

	SDL_GL_SwapWindow(displayWindow);
}

void UpdateMainMenu(float elapsed) {

	player.position.x = 0.0f;
	player.position.y = -0.85f;
	player.size.x = 0.25f;
	player.size.y = 0.25f;
}

bool e_left = false;
bool e_right = true;
int num = 1;
int restart;
int death = 0;

void UpdateGameLevel(float elapsed) {
	restart = 0;
	if (left && player.position.x >= -0.85f) { player.position.x -= elapsed*0.5f; }
	else if (right && player.position.x <= 0.85f) { player.position.x += elapsed*0.5f; }

	for (int index = 0; index < MAX_BULLETS; ++index) {
		if (bullets[index].shot) { bullets[index].position.y += elapsed*2.0f; }
		else {
			bullets[index].position.x = player.position.x; 
			bullets[bulletIndex].position.y = -2000.0f;
		}
		if (bullets[index].position.y >= 1.25f) {
			bullets[index].position.y = -2000.0f;
			bullets[index].shot = false;
		}
	}


	//determines if any of the enemies were hit by any of the bullets
	float down_ = 0.0f;
	for (size_t index = 0; index < enemies.size(); ++index) {
		for (size_t index2 = 0; index2 < enemies[index].size(); ++index2) {
			if (!enemies[index][index2].shot) {
				restart += 1;
				if (enemies[index][index2].position.y <= player.position.y + player.size.y / 2) { death += 1; }
				for (int index3 = 0; index3 < MAX_BULLETS; ++index3) {
					if (bullets[index3].shot) {
						if (bullets[index3].position.x <= enemies[index][index2].position.x) {
							if ((enemies[index][index2].position.x - (enemies[index][index2].size.x / 2)) <= (bullets[index3].position.x + (bullets[index3].size.x / 2))) {
								if (((enemies[index][index2].position.y - (enemies[index][index2].size.y / 2)) <= (bullets[index3].position.y + (bullets[index3].size.y / 2))) && ((enemies[index][index2].position.y + (enemies[index][index2].size.y / 2)) >= (bullets[index3].position.y - (bullets[index3].size.y / 2)))) {
									enemies[index][index2].shot = true;
									bullets[index3].position.x = player.position.x;
									bullets[index3].position.y = -2000.0f;
									bullets[index3].shot = false;
								}
							}
						}
						else {
							if ((enemies[index][index2].position.x + (enemies[index][index2].size.x / 2)) >= (bullets[index3].position.x - (bullets[index3].size.x / 2))) {
								if (((enemies[index][index2].position.y - (enemies[index][index2].size.y / 2)) <= (bullets[index3].position.y + (bullets[index3].size.y / 2))) && ((enemies[index][index2].position.y + (enemies[index][index2].size.y / 2)) >= (bullets[index3].position.y - (bullets[index3].size.y / 2)))) {
									enemies[index][index2].shot = true;
									bullets[index3].position.y = player.position.y;
									bullets[index3].position.y = -2000.0f;
									bullets[index3].shot = false;
								}
							}
						}
					}
				}
			}
		}
	}

	if (restart == 0 || death != 0) { state = 2; }

	for (size_t index = 0; index < enemies.size(); ++index) {
		for (size_t index2 = 0; index2 < enemies[index].size(); ++index2) {
			if (!enemies[index][index2].shot) {
				if (e_right) {
					if (enemies[index][index2].position.x >= 0.9f) {
						e_right = false;
						e_left = true;
						num = -1;
						down_ = -0.025f;
						goto finish;
					}
				}
				else if (e_left) {
					if (enemies[index][index2].position.x <= -0.9f) {
						e_left = false;
						e_right = true;
						num = 1;
						down_ = -0.025f;
						goto finish;
					}
				}
			}
		}
	}

finish:
	for (size_t index = 0; index < enemies.size(); ++index) {
		for (size_t index2 = 0; index2 < enemies[index].size(); ++index2) {
			if (!enemies[index][index2].shot) {
				enemies[index][index2].position.x += num*elapsed*0.25f;
				enemies[index][index2].position.y += down_;
			}
		}
	}
}

void ProcessMainMenuInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_Z) {
			state = 1;
		}
	}
}

int count = 0;
void ProcessGameLevelInput() {
	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_A]) {
		left = true;
		right = false;
	}
	else if (keys[SDL_SCANCODE_D]) {
		left = false;
		right = true;
	}
	else {
		left = false;
		right = false;
	}

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			count += 1;
			if (count % 2 == 1) {
				bullets[bulletIndex].shot = true;
				bullets[bulletIndex].position.y = player.position.y;
				shootBullet();
			}
		}
		else { count = 0; }
	}
}

void ProcessGameOverInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
	}
}

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

void Render(ShaderProgram& program, ShaderProgram& program2) {
	switch (state) {
		case STATE_MAIN_MENU:
			RenderMainMenu(program);
			break;
		case STATE_GAME_LEVEL:
			RenderGameLevel(program, program2);
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
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 720);
}

int main(int argc, char *argv[])
{
	Setup();
	ShaderProgram program("vertex_textured.glsl", "fragment_textured.glsl");
	ShaderProgram program2("vertex.glsl", "fragment.glsl");
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initEnemies();

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