/*
Name: Matthew Persad
NetID: mp3685
Homework #2

In this program, you are playing Pong. Player 1 uses 'W' and 'S' to move up and down, respectively.
Player 2 uses the up and down down arrow keys.
Each player's score is displayed on the screen.
The score is nine 'goals'.
You have to press spacebar in order to start every round.
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
void drawImage(ShaderProgram & program, Matrix modelMatrix, Matrix projectionMatrix, Matrix viewMatrix) {
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

class BarrierMiddle {
public:
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	void draw(ShaderProgram& program) {
		modelMatrix.identity();
		modelMatrix.Translate(0.0f, -1.0f, 0.0f);

		//draws all the images
		modelMatrix.Scale(0.05f, 0.05f, 1.0f);
		for (int index = 0; index < 15; ++index) {
			modelMatrix.Translate(0.0f, 2.5f, 0.0f);
			drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
		}
	}
};

class BackgroundTopBottom {
public:
	BackgroundTopBottom(float num) : position(num) {}
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	
	void draw(ShaderProgram& program) {
		modelMatrix.identity();

		modelMatrix.Translate(0.0f, position, 0.0f);
		modelMatrix.Scale(8.0f, 0.5f, 1.0f);
		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}
	float position;
	float width = 8.0f;
	float height = 0.5f;
};

class Player {
public:
	Player(float num) { x = num; }
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	void draw(ShaderProgram& program) {
		modelMatrix.identity();
		modelMatrix.Translate(x, y, 0.0f);
		modelMatrix.Scale(0.125f, 0.75f, 1.0f);
		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}

	float x;
	float y = 0.0f;
	float width = 0.125f;
	float height = 0.75f;
};

class Ball {
public:
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	void draw(ShaderProgram& program) {
		modelMatrix.identity();
		modelMatrix.Translate(x, y, 0.0f);
		modelMatrix.Scale(0.075f, 0.15f, 1.0f);

		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}

	float x = 0.0f;
	float y = 0.0f;
	float width = 0.075f;
	float height = 0.15f;
};

void DrawSpriteSheetSprite(ShaderProgram& program, int index, int spriteCountX, int spriteCountY, GLuint& Texture, bool test) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat texCoords[] = { u, v + spriteHeight, u + spriteWidth, v, u, v, u + spriteWidth, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight };
	float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

	Matrix modelMatrix;
	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	modelMatrix.identity();

	if (test) { modelMatrix.Translate(-1.0f, 1.0f, 0.0f); }
	else { modelMatrix.Translate(1.0f, 1.0f, 0.0f); }
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

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
}

void ProcessEvents(SDL_Event& event, bool& done, bool& start, Player& p1, Player& p2, BackgroundTopBottom& top, BackgroundTopBottom& bottom, float& elapsed, bool& p1Win, bool& p2Win) {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			start = true;
			p1Win = false;
			p2Win = false;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W]) {
		if ((p1.y + (p1.height / 2)) <= (top.position - (top.height / 2))) {
			p1.y += elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_S]) {
		if ((p1.y - (p1.height / 2)) >= (bottom.position + (bottom.height / 2))) {
			p1.y -= elapsed;
		}
	}

	if (keys[SDL_SCANCODE_UP]) {
		if ((p2.y + (p2.height / 2)) <= (top.position - (top.height / 2))) {
			p2.y += elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		if ((p2.y - (p2.height / 2)) >= (bottom.position + (bottom.height / 2))) {
			p2.y -= elapsed;
		}
	}
}

void Update(float& speed, float& elapsed, float& directionX, float& directionY, Ball& ball, BackgroundTopBottom& top, BackgroundTopBottom& bottom, Player& p1, Player& p2, bool& start, bool& win, float& lastFrameTicks, float& ticks) {
	speed += elapsed / 7.5;

	if ((ball.y + (ball.height / 2)) > (top.position - (top.height / 2))) {
		directionY *= -1;
	}

	if ((ball.y - (ball.height / 2)) < (bottom.position + (bottom.height / 2))) {
		directionY *= -1;
	}

	if ((ball.x + (ball.width / 2)) > (p2.x - (p2.width / 2))) {
		if (abs((ball.y - (ball.height / 2)) - (p2.y + (p2.height / 2))) < 0.025f) { directionY *= -1; start = false; win = true; }
		else if (abs((ball.y + (ball.height / 2)) - (p2.y - (p2.height / 2))) < 0.025f) { directionY *= -1; start = false; win = true; }
		else if ((ball.y + (ball.height / 2)) < (p2.y + (p2.height / 2))) {
			if ((ball.y + (ball.height / 2)) > (p2.y - (p2.height / 2))) {
				directionX *= -1;
			}
		}
		else if ((ball.y - (ball.height / 2)) > (p2.y - (p2.height / 2))) {
			if ((ball.y - (ball.height / 2)) < (p2.y + (p2.height / 2))) {
				directionX *= -1;
			}
		}
	}

	if ((ball.x - (ball.width / 2)) < (p1.x + (p1.width / 2))) {
		if (abs((ball.y - (ball.height / 2)) - (p1.y + (p1.height / 2))) < 0.025f) { directionY *= -1; start = false; win = true; }
		else if (abs((ball.y + (ball.height / 2)) - (p1.y - (p1.height / 2))) < 0.025f) { directionY *= -1; start = false; win = true; }
		else if ((ball.y + (ball.height / 2)) < (p1.y + (p1.height / 2))) {
			if ((ball.y + (ball.height / 2)) > (p1.y - (p1.height / 2))) {
				directionX *= -1;
			}
		}
		else if ((ball.y - (ball.height / 2)) > (p1.y - (p1.height / 2))) {
			if ((ball.y - (ball.height / 2)) < (p1.y + (p1.height / 2))) {
				directionX *= -1;
			}
		}
	}
}

void drawWinScreen(ShaderProgram& program, GLuint& text, bool& p1Win, bool& p2Win, int spriteCountX, int spriteCountY){
	float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
	float num;
	if (p1Win) { num = 1; }
	else { num = -0.33; }

	Matrix modelMatrix;
	Matrix projectionMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	modelMatrix.identity();
	modelMatrix.Translate(-2.5f*num, 0.0f, 0.0f);
	modelMatrix.Scale(0.25f, 0.25f, 1.0f);

	std::vector<int> win_text = { 89, 111, 117, 87, 111, 110, 33};

	for (size_t index = 0; index < win_text.size(); ++index) {
		float u = (float)(((int)win_text[index]) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)win_text[index]) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0 / (float)spriteCountX;
		float spriteHeight = 1.0 / (float)spriteCountY;
		GLfloat texCoords[] = { u, v + spriteHeight, u + spriteWidth, v, u, v, u + spriteWidth, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight };

		if (index == 3) { modelMatrix.Translate(-3.25f, -1.25f, 0.0f); }
		modelMatrix.Translate(1.0f, 0.0f, 0.0f);

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

	std::vector<int> again_text = { 84, 114, 121, 65, 103, 97, 105, 110, 63 };

	if (p1Win) { num = -0.33; }
	else { num = 1; }

	modelMatrix.identity();
	modelMatrix.Translate(-2.5f*num, 0.0f, 0.0f);
	modelMatrix.Scale(0.25f, 0.25f, 1.0f);

	for (size_t index = 0; index < again_text.size(); ++index) {
		float u = (float)(((int)again_text[index]) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)again_text[index]) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0 / (float)spriteCountX;
		float spriteHeight = 1.0 / (float)spriteCountY;
		GLfloat texCoords[] = { u, v + spriteHeight, u + spriteWidth, v, u, v, u + spriteWidth, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight };

		if (index == 3) { modelMatrix.Translate(-3.25f, -1.25f, 0.0f); }
		modelMatrix.Translate(1.0f, 0.0f, 0.0f);

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

void Render(ShaderProgram& program, ShaderProgram& program2, GLuint& text, Ball& ball, Player& p1, Player& p2, BackgroundTopBottom& top, BackgroundTopBottom& bottom, BarrierMiddle& mid, float& elapsed, float& directionX, float& directionY, float& speed, bool& start, bool& win, std::vector<int>& runAnimation, int& currentIndex1, int& currentIndex2, bool& p1Win, bool& p2Win) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (start) {
		ball.x += elapsed*directionX*speed;
		ball.y += elapsed*directionY*speed;
	}

	if (win) {
		ball.x += elapsed*directionX*speed;
		ball.y += elapsed*directionY*speed;
	}

	mid.draw(program);
	top.draw(program);
	bottom.draw(program);
	p1.draw(program);
	p2.draw(program);

	if (ball.x <= -1.1f || ball.x >= 1.1f) {
		if (ball.x <= -1.1f) { currentIndex2 += 1; }
		else if (ball.x >= 1.1f) { currentIndex1 += 1; }
		ball.x = 0.0f;
		ball.y = 0.0f;
		start = false;
		speed = 1.0f;
		win = false;
		if (currentIndex1 == 9 || currentIndex2 == 9) { 
			if (currentIndex1 == 9) { p1Win = true; }
			else { p2Win = true; }
			currentIndex1 = 0;
			currentIndex2 = 0; 
		}

	}
	DrawSpriteSheetSprite(program2, runAnimation[currentIndex1], 16, 16, text, true);
	DrawSpriteSheetSprite(program2, runAnimation[currentIndex2], 16, 16, text, false);
	ball.draw(program);

	if (p1Win || p2Win) { drawWinScreen(program2, text, p1Win, p2Win, 16, 16); }

	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
	Setup();

	ShaderProgram program("vertex.glsl", "fragment.glsl");
	ShaderProgram program2("vertex_textured.glsl", "fragment_textured.glsl");

	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;

	float directionX = cos(45.0f * 3.14159 / 180.0f);
	float directionY = sin(45.0f * 3.14159 / 180.0f);

	bool start = false;
	bool win = false;
	
	Player p1(-1.0f);
	Player p2(1.0f);

	BarrierMiddle mid;
	BackgroundTopBottom top(1.125f);
	BackgroundTopBottom bottom(-1.125f);

	int test_x = rand() % 100;
	int test_y = rand() % 100;

	if (test_x < 50) { directionX *= -1; }
	if (test_y < 50) { directionY *= -1; }

	Ball ball;

	float speed = 1.0f;

	GLuint text = LoadTexture("pixel_font.png");

	std::vector<int> runAnimation = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 };
	int currentIndex1 = 0;
	int currentIndex2 = 0;

	bool p1Win = false;
	bool p2Win = false;
	bool startOver = false;

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		ProcessEvents(event, done, start, p1, p2, top, bottom, elapsed, p1Win, p2Win);
		
		if (start) {
			Update(speed, elapsed, directionX, directionY, ball, top, bottom, p1, p2, start, win, lastFrameTicks, ticks);
		}

		Render(program, program2, text, ball, p1, p2, top, bottom, mid, elapsed, directionX, directionY, speed, start, win, runAnimation, currentIndex1, currentIndex2, p1Win, p2Win);
	}

	SDL_Quit();
	return 0;
}