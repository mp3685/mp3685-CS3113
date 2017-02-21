/*
Name: Matthew Persad
NetID: mp3685
Homework #1

In this program, you control the character on the left (Riku) with 'a' and 'd'.
You are fighting the computer (Sora).
You are able to summon fire by pressing the return key.
You can also do a special summon by pressing the spacebar.
*/

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

//function to draw the image
void drawImage(ShaderProgram & program, Matrix& modelMatrix, Matrix& projectionMatrix, Matrix& viewMatrix, GLuint Texture) {
	program.setModelMatrix(modelMatrix);
	program.setProjectionMatrix(projectionMatrix);
	program.setViewMatrix(viewMatrix);
	glBindTexture(GL_TEXTURE_2D, Texture);
	float vertices[] = { -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0 };

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

//draws background grass
void drawBackground(ShaderProgram& program, Matrix& projectionMatrixGrass, Matrix& modelMatrixGrass, Matrix& viewMatrixGrass, GLuint grassTexture, float num) {
	modelMatrixGrass.Translate(num, -1.5f, 0);
	modelMatrixGrass.Scale(0.75f, 0.75f, 0);

	modelMatrixGrass.Scale(4.728f, 1.0f, 0);

	drawImage(program, modelMatrixGrass, projectionMatrixGrass, viewMatrixGrass, grassTexture);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);

	ShaderProgram program("vertex_textured.glsl", "fragment_textured.glsl");

	GLuint computerTexture = LoadTexture("sora.png");
	GLuint playerTexture = LoadTexture("riku_replica.png");
	GLuint summonTexture = LoadTexture("kirby.png");
	GLuint grassTexture = LoadTexture("grass.png");
	GLuint fireTexture = LoadTexture("fireball.png");

	Matrix projectionMatrixGrass;
	Matrix modelMatrixGrass;
	Matrix viewMatrixGrass;

	Matrix projectionMatrixFire;
	Matrix modelMatrixFire;
	Matrix viewMatrixFire;

	Matrix projectionMatrixComputer;
	Matrix modelMatrixComputer;
	Matrix viewMatrixComputer;

	Matrix projectionMatrixPlayer;
	Matrix modelMatrixPlayer;
	Matrix viewMatrixPlayer;

	Matrix projectionMatrixSummon;
	Matrix modelMatrixSummon;
	Matrix viewMatrixSummon;
	
	projectionMatrixComputer.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	projectionMatrixPlayer.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	projectionMatrixSummon.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	projectionMatrixGrass.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	projectionMatrixFire.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	
	glUseProgram(program.programID);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;

	float angle = 45.0f * (3.14159/180);
	float distanceComputer = 0.5f;
	float distancePlayer = 0.0f;
	float scaleBigAndSmall = 1.0f;
	float scaleBigAndSmall2 = 1.0f;
	float positionX = -2.5f;
	float positionY = 0.95f;
	float positionX2 = -2.5f;
	float directionX = cos(-23.5f * 3.14159 / 180.0f);
	float directionY = sin(-23.5f * 3.14159 / 180.0f);
	bool testComputer = true;
	bool testPlayer = true;
	bool testBigAndSmall = true;
	bool testBigAndSmall2 = true;
	bool spawn = false;
	bool spawn_fire = false;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					spawn = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					spawn_fire = true;
				}
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		angle += elapsed;

		const Uint8* keys = SDL_GetKeyboardState(NULL);

		//calculates position for player
		if (keys[SDL_SCANCODE_D]) {
			if (!(distancePlayer >= 2.5f)) {
				distancePlayer += elapsed * 2.0f;
			}
		}
		else if (keys[SDL_SCANCODE_A]) {
			if (!(distancePlayer <= -2.5f)) {
				distancePlayer -= elapsed * 2.0f;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.74902f, 0.847059f, 0.847059f, 1.0f);

		modelMatrixComputer.identity();
		modelMatrixPlayer.identity();
		modelMatrixSummon.identity();
		modelMatrixGrass.identity();
		modelMatrixFire.identity();

		//calculates position for computer
		if (distanceComputer >= 2.5f || distanceComputer <= 0.49f) {
			if (distanceComputer > 0.49f) { testComputer = false; }
			else { testComputer = true; }
		}

		if (testComputer) {
			distanceComputer += elapsed;
		}
		else {
			distanceComputer -= elapsed;
		}

		modelMatrixComputer.Translate(distanceComputer, -1.1f, 0);
		modelMatrixPlayer.Translate(distancePlayer, -1.1f, 0);

		//calculates position for summon
		if (positionX >= 2.75f && positionY <= -2.0f) { 
			spawn = false;
			positionX = -2.5f;
			positionY = 0.95f;
		}

		if (spawn) {
			if (abs(scaleBigAndSmall) >= 1.25f) {
				if (scaleBigAndSmall > 0) { testBigAndSmall = false; }
				else { testBigAndSmall = true; }
			}

			if (testBigAndSmall) {
				scaleBigAndSmall += elapsed;
			}
			else {
				scaleBigAndSmall -= elapsed;
			}
			positionX += directionX*elapsed*5.0f;
			positionY += directionY*elapsed*5.0f;
			modelMatrixSummon.Translate(positionX, positionY, 0);
			modelMatrixSummon.Rotate(angle * 100);
		}

		//calculates position for fire
		if (spawn_fire) {
			if (abs(scaleBigAndSmall2) >= 1.25f) {
				if (scaleBigAndSmall2 > 0) { testBigAndSmall2 = false; }
				else { testBigAndSmall2 = true; }
			}

			if (testBigAndSmall2) {
				scaleBigAndSmall2 += elapsed;
			}
			else {
				scaleBigAndSmall2 -= elapsed;
			}
			positionX2 += directionX*elapsed*7.5f;
			modelMatrixFire.Translate(positionX2+directionX*elapsed*7.5f, -1.0f, 0);
		}
		else { positionX2 = distancePlayer; }

		if (positionX2 >= 2.75f) {
			spawn_fire = false;
			positionX2 = -2.5f;
		}

		//draws all the images
		drawBackground(program, projectionMatrixGrass, modelMatrixGrass, viewMatrixGrass, grassTexture, 0);
		if (spawn) { drawImage(program, modelMatrixSummon, projectionMatrixSummon, viewMatrixSummon, summonTexture); }
		if (spawn_fire) { drawImage(program, modelMatrixFire, projectionMatrixFire, viewMatrixFire, fireTexture); }
		drawImage(program, modelMatrixComputer, projectionMatrixComputer, viewMatrixComputer, computerTexture);
		drawImage(program, modelMatrixPlayer, projectionMatrixPlayer, viewMatrixPlayer, playerTexture);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}