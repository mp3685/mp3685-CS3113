/*
Name: Matthew Persad
NetID: mp3685
Homework #5

In this project, you are watcing a demo of the SAT collision function.
There are four boxes, and the SAT collision function calculates and moves the boxes away from each other.
The biggest box (Entity two) is constantly moving, and all other boxes are being pushed away as they
collide with it. But if two other Entities do happen to touch (Entity one and Entity three), they just go
in the opposite directions.

You can press "R" to replay the collision once all the boxes go off-screen.
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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

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
	Vector3(const Vector3& test) {
		x = test.x;
		y = test.y;
		z = test.z;
	}

	void operator=(const Vector3& test) {
		this->x = test.x;
		this->y = test.y;
		this->z = test.z;
	}

	float length() {
		return sqrtf((x*x) + (y*y));
	}

	void normalize() {
		float length = sqrtf((x*x) + (y*y));
		x /= length;
		y /= length;
	}

	Vector3 operator*(const Matrix& v) {
		Vector3 temp;
		temp.x = v.m[0][0] * x + v.m[1][0] * y + v.m[2][0] * 0 + v.m[3][0] * 1.0f;
		temp.y = v.m[0][1] * x + v.m[1][1] * y + v.m[2][1] * 0 + v.m[3][1] * 1.0f;

		return temp;
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
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		drawImage(program, modelMatrix, projectionMatrix, viewMatrix);
	}

	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	float rotation;
	SheetSprite sprite;
	Matrix modelMatrix;
	int dir;
	Matrix matrix;
};

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector3> &points1, const std::vector<Vector3> &points2, Vector3 &penetration) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];

	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = 2*dist - ((e1Width + e2Width) / 2.0);

	if (p >= 0) {
		return false;
	}

	float penetrationMin1 = e1Max - e2Min;
	float penetrationMin2 = e2Max - e1Min;

	float penetrationAmount = penetrationMin1;
	if (penetrationMin2 < penetrationAmount) {
		penetrationAmount = penetrationMin2;
	}

	penetration.x = normalX * penetrationAmount;
	penetration.y = normalY * penetrationAmount;

	return true;
}

bool penetrationSort(Vector3 &p1, Vector3 &p2) {
	return p1.length() < p2.length();
}

bool checkSATCollision(const std::vector<Vector3> &e1Points, const std::vector<Vector3> &e2Points, Vector3 &penetration) {
	std::vector<Vector3> penetrations;
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;
		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}
		Vector3 penetration1;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration1);
		if (!result) {
			return false;
		}
		penetrations.push_back(penetration1);
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		Vector3 penetration2;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration2);

		if (!result) {
			return false;
		}
		penetrations.push_back(penetration2);
	}
	std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
	penetration = penetrations[0];

	Vector3 e1Center;
	for (int i = 0; i < e1Points.size(); i++) {
		e1Center.x += e1Points[i].x;
		e1Center.y += e1Points[i].y;
	}
	e1Center.x /= (float)e1Points.size();
	e1Center.y /= (float)e1Points.size();

	Vector3 e2Center;
	for (int i = 0; i < e2Points.size(); i++) {
		e2Center.x += e2Points[i].x;
		e2Center.y += e2Points[i].y;
	}
	e2Center.x /= (float)e2Points.size();
	e2Center.y /= (float)e2Points.size();

	Vector3 ba;
	ba.x = e1Center.x - e2Center.x;
	ba.y = e1Center.y - e2Center.y;
	
	cout << penetration.x << "\t" << penetration.y << endl;
	cout << e1Center.x << "\t" << e2Center.x << endl;
	cout << e1Center.y << "\t" << e2Center.y << endl;
	cout << "THIS: " << penetration.x << "\t" << penetration.y << endl;
	if ((penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
		penetration.x *= -1.0f;
		penetration.y *= -1.0f;
		cout << "THIS2: " << penetration.x << "\t" << penetration.y << endl;
	}
	return true;
}

float lastFrameTicks = 0.0f;

SDL_Event event;
bool done = false;

Entity one;
Entity two;
Entity three;
Entity four;

vector<Vector3> e1Points;
vector<Vector3> e2Points;
vector<Vector3> e3Points;
vector<Vector3> e4Points;

void fillVector(const Entity& e, vector<Vector3>& points) {
	vector<float> x;
	vector<float> y;

	x.push_back(-1.0f);
	x.push_back(1.0f);
	x.push_back(1.0f);
	x.push_back(-1.0f);

	y.push_back(1.0f);
	y.push_back(1.0f);
	y.push_back(-1.0f);
	y.push_back(-1.0f);

	for (size_t index = 0; index < x.size(); ++index) {
		Vector3 temp;
		temp.x = x[index];
		temp.y = y[index];
		temp = temp*e.matrix;
		points.push_back(temp);
	}
}

void initEntities() {
	one.position.x = -0.5f;
	one.position.y = -0.5f;
	one.size.x = 0.25f;
	one.size.y = 0.25f;
	one.rotation = 45 * acos(-1) / 180;
	one.dir = 1;

	one.modelMatrix.identity();
	one.modelMatrix.Translate(one.position.x, one.position.y, 0.0f);
	one.modelMatrix.Rotate(one.rotation);
	one.modelMatrix.Scale(one.size.x, one.size.y, 1.0f);

	one.matrix.identity();
	Matrix translate1;
	translate1.setPosition(one.position.x, one.position.y, 0.0f);
	Matrix rotate1;
	rotate1.setRoll(one.rotation);
	Matrix scale1;
	scale1.setScale(one.size.x, one.size.y, 1.0f);
	Matrix transform1 = scale1 * rotate1 * translate1;
	one.matrix = transform1;

	two.position.x = 0.0f;
	two.position.y = -0.5f;
	two.size.x = 0.5f;
	two.size.y = 0.5f;
	two.rotation = 30 * acos(-1) / 180;
	two.dir = 1;

	two.modelMatrix.identity();
	two.modelMatrix.Translate(two.position.x, two.position.y, 0.0f);
	two.modelMatrix.Rotate(two.rotation);
	two.modelMatrix.Scale(two.size.x, two.size.y, 1.0f);

	two.matrix.identity();
	Matrix translate2;
	translate2.setPosition(two.position.x, two.position.y, 0.0f);
	Matrix rotate2;
	rotate2.setRoll(two.rotation);
	Matrix scale2;
	scale2.setScale(two.size.x, two.size.y, 1.0f);
	Matrix transform2 = scale2 * rotate2 * translate2;
	two.matrix = transform2;

	three.position.x = 0.5f;
	three.position.y = -0.25f;
	three.size.x = 0.125f;
	three.size.y = 0.125f;
	three.rotation = 60 * acos(-1) / 180;
	three.dir = 1;

	three.modelMatrix.identity();
	three.modelMatrix.Translate(three.position.x, three.position.y, 0.0f);
	three.modelMatrix.Rotate(three.rotation);
	three.modelMatrix.Scale(three.size.x, three.size.y, 1.0f);

	three.matrix.identity();
	Matrix translate3;
	translate3.setPosition(three.position.x, three.position.y, 0.0f);
	Matrix rotate3;
	rotate3.setRoll(three.rotation);
	Matrix scale3;
	scale3.setScale(three.size.x, three.size.y, 1.0f);
	Matrix transform3 = scale3 * rotate3 * translate3;
	three.matrix = transform3;

	four.position.x = -0.5f;
	four.position.y = 0.5f;
	four.size.x = 0.25f;
	four.size.y = 0.25f;
	four.rotation = 45 * acos(-1) / 180;
	four.dir = 1;
	
	four.modelMatrix.identity();
	four.modelMatrix.Translate(four.position.x, four.position.y, 0.0f);
	four.modelMatrix.Rotate(four.rotation);
	four.modelMatrix.Scale(four.size.x, four.size.y, 1.0f);

	four.matrix.identity();
	Matrix translate4;
	translate4.setPosition(four.position.x, four.position.y, 0.0f);
	Matrix rotate4;
	rotate4.setRoll(four.rotation);
	Matrix scale4;
	scale4.setScale(four.size.x, four.size.y, 1.0f);
	Matrix transform4 = scale4 * rotate4 * translate4;
	four.matrix = transform4;
}

//selects GAME_LEVEL functions
int state = 1;

void RenderMainMenu(ShaderProgram& program) {
	//nothing
}

void RenderGameLevel(ShaderProgram& program) {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);

	one.Draw(program);
	two.Draw(program);
	three.Draw(program);
	four.Draw(program);

	SDL_GL_SwapWindow(displayWindow);
}

void RenderGameOver(ShaderProgram& program) {
	//nothing
}

void UpdateMainMenu(float& elapsed) {
	//nothing
}

void UpdateGameLevel(float elapsed) {
	Matrix temp_translate;

	one.modelMatrix.Translate((elapsed)* one.dir, -(elapsed)* one.dir, 0.0f);
	temp_translate.identity();
	temp_translate.setPosition((elapsed)* one.dir, -(elapsed)* one.dir, 0.0f);
	one.matrix = temp_translate * one.matrix;

	temp_translate.identity();
	two.modelMatrix.Translate((elapsed)* two.dir*0.5, (elapsed)* two.dir*0.5, 0.0f);
	temp_translate.setPosition((elapsed)* two.dir*0.5, (elapsed)* two.dir*0.5, 0.0f);
	two.matrix = temp_translate * two.matrix;

	temp_translate.identity();
	three.modelMatrix.Translate(-(elapsed)* three.dir*0.5, (elapsed)* three.dir*0.5, 0.0f);
	temp_translate.setPosition(-(elapsed)* three.dir*0.5, (elapsed)* three.dir*0.5, 0.0f);
	three.matrix = temp_translate * three.matrix;

	temp_translate.identity();
	four.modelMatrix.Translate((elapsed)* four.dir*0.5, -(elapsed)* four.dir*0.5, 0.0f);
	temp_translate.setPosition((elapsed)* four.dir*0.5, -(elapsed)* four.dir*0.5, 0.0f);
	four.matrix = temp_translate * four.matrix;

	//fills the vectors of edges for the SAT collision function
	fillVector(one, e1Points);
	fillVector(two, e2Points);
	fillVector(three, e3Points);
	fillVector(four, e4Points);

	Vector3 penetration12;
	Vector3 penetration13;
	Vector3 penetration23;
	Vector3 penetration24;

	//test for Entity one and Entity two
	if (checkSATCollision(e1Points, e2Points, penetration12)) {
		if (penetration12.x < 0) { penetration12.x *= -1; }
		if (penetration12.y < 0) { penetration12.y *= -1; }
		temp_translate.identity();
		one.modelMatrix.Translate(-penetration12.x / 2, penetration12.y/2, 0.0f);
		temp_translate.setPosition(-penetration12.x / 2, penetration12.y/2, 0.0f);
		one.matrix = temp_translate * one.matrix;
	}
	
	//test for Entity one and Entity three
	if (checkSATCollision(e1Points, e3Points, penetration13)) {
		if (penetration13.x < 0) { penetration13.x *= -1; }
		if (penetration13.y < 0) { penetration13.y *= -1; }
		temp_translate.identity();
		one.modelMatrix.Translate(-penetration13.x / 2, penetration13.y/2, 0.0f);
		temp_translate.setPosition(-penetration13.x / 2, penetration13.y/2, 0.0f);
		one.matrix = temp_translate * one.matrix;
		one.dir *= -1;

		temp_translate.identity();
		three.modelMatrix.Translate(penetration13.x / 2, -penetration13.y/2, 0.0f);
		temp_translate.setPosition(penetration13.x / 2, -penetration13.y/2, 0.0f);
		three.matrix = temp_translate * three.matrix;
		three.dir *= -1; 
	}
	
	//test for Entity two and Entity two
	if (checkSATCollision(e2Points, e3Points, penetration23)) {
		if (penetration23.x < 0) { penetration23.x *= -1; }
		if (penetration23.y < 0) { penetration23.y *= -1; }

		temp_translate.identity();
		three.dir *= -1;
		three.modelMatrix.Translate((penetration23.x/2), -(penetration23.y/2), 0.0f);
		temp_translate.setPosition((penetration23.x/2), -(penetration23.y/2), 0.0f);
		three.matrix = temp_translate * three.matrix;
	}

	//test for Entity two and Entity four
	if (checkSATCollision(e2Points, e4Points, penetration24)) {
		if (penetration24.x < 0) { penetration24.x *= -1; }
		if (penetration24.y < 0) { penetration24.y *= -1; }

		temp_translate.identity();
		four.modelMatrix.Translate(-penetration24.x / 2, penetration24.y/2, 0.0f);
		temp_translate.setPosition(-penetration24.x / 2, penetration24.y/2, 0.0f);
		four.matrix = temp_translate * four.matrix;
	}

	//clears the vectors of the edges
	e1Points.clear();
	e2Points.clear();
	e3Points.clear();
	e4Points.clear();
}

void ProcessMainMenuInput() {
	//nothing
}

void ProcessGameLevelInput() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			done = true;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
			initEntities();
		}
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
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 600, 600);
}

int main(int argc, char *argv[])
{
	Setup();
	ShaderProgram program("vertex.glsl", "fragment.glsl");
	glUseProgram(program.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initEntities();

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

	SDL_Quit();
	return 0;
}