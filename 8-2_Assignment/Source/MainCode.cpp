#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <math.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

// Global paddle position variable (manual control)
float paddleX = 0.0f;
const float PADDLE_SPEED = 0.04f; // Speed at which the paddle moves
const float PADDLE_WIDTH = 0.4f; // Width of the paddle

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int hitsRemaining; // Tracks multi-hit degradation for destructible bricks

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
		// Reflective blocks are made static and destructible blocks are made dynamic (3 strikes to destroy)
		hitsRemaining = (bt == DESTRUCTABLE) ? 3 : 999;
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle {
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;  // Base speed
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	bool hashHitWall = false;  // Tracking boundary interaction states
	bool isAlive = true;  // Circle's active state

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void invertTrajectory()
	{
		// Safe mapping of direction to its opposite
		if (direction == 1) direction = 3; // up to down
		else if (direction == 2) direction = 4; // right to left
		else if (direction == 3) direction = 1; // down to up
		else if (direction == 4) direction = 2; // left to right
		else if (direction == 5) direction = 8; // up-right to down-left
		else if (direction == 6) direction = 7; // up-left to down-right
		else if (direction == 7) direction = 6; // down-right to up-left
		else if (direction == 8) direction = 5; // down-left to up-right
		else {
			direction = GetRandomDirection(); // Fallback for unexpected values
		}
	}

	// Updating properties once the circle reflects off an edge
	void applyWallImpactPhysics() {
		if (!hashHitWall)
		{
			hashHitWall = true;
			speed = 0.035f;  // Slightly increase speed after hitting a wall
		}
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->onoff == OFF) return;

		double halfside = brk->width / 2.0;

			// Axis-aligned bounding box (AABB) collision detection
			if ((x + radius >= brk->x - halfside && x - radius <= brk->x + halfside) && 
				(y + radius >= brk->y - halfside && y - radius <= brk->y + halfside))
			{
				// Physics trajectory adjustment based on collision side
				invertTrajectory();

				if (brk->brick_type == DESTRUCTABLE) {
					brk->hitsRemaining--;
					// Structural color mutation based on remaining hits
					brk->red *= 0.5f;
					brk->green *= 0.5f;
					brk->blue *= 0.5f;
					
					if (brk->hitsRemaining <= 0) {
						brk->onoff = OFF; // Deactivate brick after all hits
					}
				}
			}
		}

	// Check collision against the paddle (manual control)
	void CheckPaddleCollision() {
		float halfPaddle = PADDLE_WIDTH / 2.0f;
		float paddleTop = -0.85f; // Assuming paddle is at y = -0.85
		float paddleBottom = paddleTop - -0.90f; // Paddle height of -0.90

		if (x >= paddleX - halfPaddle && x <= paddleX + halfPaddle) {
			if (y - radius <= paddleTop && y + radius >= paddleBottom) {
				// Bounce off the paddle
				if (direction == 3) direction = 1;
				else if (direction == 7) direction = 5;
				else if (direction == 8) direction = 6;
				y = paddleTop + radius; // Adjust position to avoid sticking
			}
		}
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y < 1.0f - radius)
			{
				y += speed;
			}
			else
			{
				direction = (direction == 1) ? 3 : (direction == 5) ? 7 : 8;
			}
		}

		else if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y > -1.0f + radius)
			{
				y -= speed;
			}
			else
			{
				direction = (direction == 3) ? 1 : (direction == 7) ? 5 : 6;
				applyWallImpactPhysics();
			}
		}
		// Horizontal movement
		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1.0f - radius) {
				x += speed;
			}
			else
			{
				direction = (direction == 3) ? 4 : ((direction == 5) ? 6 : 8);
				applyWallImpactPhysics();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1.0f + radius) {
				x -= speed;
			}
			else
			{
				direction = (direction == 4) ? 2 : ((direction == 6) ? 5 : 7);
				applyWallImpactPhysics();
			}
		}
	}

	void DrawCircle()
	{
		if (!isAlive) return;  // Skip rendering if the circle is inactive
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;

void handleCircleCollisions() {
	for (size_t i = 0; i < world.size(); i++) {
		for (size_t j = i + 1; j < world.size(); j++) {
			if (!world[i].isAlive || !world[j].isAlive) continue; // Skip inactive circles

			float dx = world[i].x - world[j].x;
			float dy = world[i].y - world[j].y;
			float distance = sqrt(dx * dx + dy * dy);

			// If circles are colliding
			if (distance < (world[i].radius + world[j].radius)) {
				// First circle absorbs the second circle and grows in size
				world[i].radius += 0.02f; // Increase radius of the first circle
				world[i].red = 1.0f; // Change color to bright pinkish purple
				world[i].green = 0.0f;
				world[i].blue = 1.0f;

				world[j].isAlive = false; // Mark the second circle as inactive and destroy
			}
		}
	}
}

// Drawing the custom brick-based paddle structure
void drawPaddle() {
	float halfWidth = PADDLE_WIDTH / 2.0f;
	glColor3f(1.0f, 1.0f, 1.0f); // Paddle color (white)
	glBegin(GL_POLYGON);
	glVertex2f(paddleX - halfWidth, -0.85f); // Bottom
	glVertex2f(paddleX + halfWidth, -0.85f); // Bottom
	glVertex2f(paddleX + halfWidth, -0.90f); // Top
	glVertex2f(paddleX - halfWidth, -0.90f); // Top
	glEnd();
}

int main(void) {
	srand(static_cast<unsigned int>(time(NULL)));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment - 2D Animation", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Visually unique brick arrangement
	vector<Brick> bricks;
	bricks.push_back(Brick(REFLECTIVE, -0.6f, 0.6f, 0.25f, 1.0f, 0.5f, 0.0f));
	bricks.push_back(Brick(DESTRUCTABLE, -0.2f, 0.6f, 0.2f, 1.0f, 1.0f, 0.0f));
	bricks.push_back(Brick(DESTRUCTABLE, 0.2f, 0.6f, 0.2f, 1.0f, 1.0f, 0.0f));
	bricks.push_back(Brick(REFLECTIVE, 0.6f, 0.6f, 0.25f, 1.0f, 0.5f, 0.0f));

	// Middle bricks
	bricks.push_back(Brick(DESTRUCTABLE, -0.4f, 0.3f, 0.2f, 0.0f, 1.0f, 1.0f));
	bricks.push_back(Brick(REFLECTIVE, 0.0f, 0.3f, 0.3f, 0.8f, 0.2f, 0.8f));
	bricks.push_back(Brick(DESTRUCTABLE, 0.4f, 0.3f, 0.2f, 0.0f, 1.0f, 1.0f));

	// Seed initial starting circles
	world.push_back(Circle(0.0f, -0.4f, 0.05f, 5, 0.05f, 1.0f, 0.0f, 0.0f));
	world.push_back(Circle(-0.3f, -0.2f, 0.04f, 6, 0.04f, 0.0f, 1.0f, 0.0f));

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// Process step checks and calculations
		handleCircleCollisions();

		//Movement
		for (size_t i = 0; i < world.size(); i++) {
			if (!world[i].isAlive) continue; // Skip inactive circles

			// Checking boundaries vs all static blocks
			for (size_t b = 0; b < bricks.size(); b++) {
				world[i].CheckCollision(&bricks[b]);
			}

			world[i].CheckPaddleCollision(); // Check against the paddle
			world[i].MoveOneStep();
			world[i].DrawCircle();
		}

		for(size_t b = 0; b < bricks.size(); b++) {
			bricks[b].drawBrick();
		}

		drawPaddle(); // Render the paddle

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Paddle movement control with A key
	if (glfwGetKey(window, 65) == GLFW_PRESS)
	{
		// Clamp to left screen boundary
		if (paddleX - PADDLE_WIDTH / 2.0f > 1.0f)
			paddleX -= PADDLE_SPEED;
	}

	// Move paddle left with left arrow key
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		// Clamp to left screen boundary
		if (paddleX - PADDLE_WIDTH / 2.0f > -1.0f)
			paddleX -= PADDLE_SPEED;
	}

	// Right movement with D key
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		// Clamp to right screen boundary
		if (paddleX + PADDLE_WIDTH / 2.0f < 1.0f)
			paddleX += PADDLE_SPEED;
	}

	// Spacebar to spawn a new circle with random color and direction
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);
	}
}