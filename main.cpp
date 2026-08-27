#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include<cmath>

// Window Dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Game States
enum GameState { START, PLAYING, GAME_OVER };
GameState currentState = START;

// Bird Properties
float birdX = 150.0f;
float birdY = 300.0f;
float birdRadius = 15.0f;
float velocity = 0.0f;
float gravity = -0.4f;
float jumpStrength = 7.5f;

// Score
int score = 0;

// Pipe (Obstacle Wall) Structure
struct Pipe {
    float x;
    float topHeight;    // Height from top edge down
    float bottomHeight; // Height from bottom edge up
    float width;
    bool passed;
};

std::vector<Pipe> pipes;
float pipeSpeed = 3.0f;
float pipeSpawnTimer = 0;

// Function to draw text on screen
void drawText(float x, float y, std::string text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// Spawn a new pair of top and bottom pipes
void spawnPipe() {
    Pipe p;
    p.x = WINDOW_WIDTH;
    p.width = 60.0f;

    // Randomize gap position
    float gap = 160.0f; // Gap size for bird to fly through
    float minHeight = 80.0f;
    float maxHeight = WINDOW_HEIGHT - gap - minHeight;

    p.bottomHeight = minHeight + rand() % (int)(maxHeight - minHeight);
    p.topHeight = WINDOW_HEIGHT - (p.bottomHeight + gap);
    p.passed = false;

    pipes.push_back(p);
}

// Reset Game
void resetGame() {
    birdY = 300.0f;
    velocity = 0.0f;
    score = 0;
    pipes.clear();
    spawnPipe();
    currentState = PLAYING;
}

// Collision Detection (Circle vs Rectangle)
bool checkCollision(float bx, float by, float br, float px, float py, float pw, float ph) {
    float closestX = (bx < px) ? px : ((bx > px + pw) ? px + pw : bx);
    float closestY = (by < py) ? py : ((by > py + ph) ? py + ph : by);

    float distX = bx - closestX;
    float distY = by - closestY;

    return (distX * distX + distY * distY) < (br * br);
}

// Update Loop (60 FPS)
void update(int value) {
    if (currentState == PLAYING) {
        // Apply Gravity
        velocity += gravity;
        birdY += velocity;

        // Check ground/ceiling collision
        if (birdY - birdRadius <= 0 || birdY + birdRadius >= WINDOW_HEIGHT) {
            currentState = GAME_OVER;
        }

        // Handle Pipes
        pipeSpawnTimer += 1.0f;
        if (pipeSpawnTimer >= 90) { // Spawn new pipe every ~1.5 seconds
            spawnPipe();
            pipeSpawnTimer = 0;
        }

        for (size_t i = 0; i < pipes.size(); i++) {
            pipes[i].x -= pipeSpeed;

            // Check collision with Bottom Pipe
            if (checkCollision(birdX, birdY, birdRadius, pipes[i].x, 0, pipes[i].width, pipes[i].bottomHeight)) {
                currentState = GAME_OVER;
            }

            // Check collision with Top Pipe
            if (checkCollision(birdX, birdY, birdRadius, pipes[i].x, WINDOW_HEIGHT - pipes[i].topHeight, pipes[i].width, pipes[i].topHeight)) {
                currentState = GAME_OVER;
            }

            // Score Increment
            if (!pipes[i].passed && pipes[i].x + pipes[i].width < birdX) {
                pipes[i].passed = true;
                score++;
            }
        }

        // Remove off-screen pipes
        if (!pipes.empty() && pipes[0].x + pipes[0].width < 0) {
            pipes.erase(pipes.begin());
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

// Render/Display Function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Draw Background (Sky Blue)
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);

    // 2. Draw Pipes (Green Walls)
    for (const auto& p : pipes) {
        glColor3f(0.1f, 0.8f, 0.2f);

        // Bottom Pipe
        glRectf(p.x, 0, p.x + p.width, p.bottomHeight);

        // Top Pipe
        glRectf(p.x, WINDOW_HEIGHT - p.topHeight, p.x + p.width, WINDOW_HEIGHT);
    }

    // 3. Draw Bird (Yellow Circle)
    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i += 10) {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(birdX + cos(rad) * birdRadius, birdY + sin(rad) * birdRadius);
    }
    glEnd();

    // 4. Draw Score
    drawText(20, WINDOW_HEIGHT - 40, "Score: " + std::to_string(score));

    // 5. Draw Game State Screens
    if (currentState == START) {
        drawText(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2, "PRESS SPACE TO START", GLUT_BITMAP_HELVETICA_18);
    } else if (currentState == GAME_OVER) {
        drawText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 20, "GAME OVER!", GLUT_BITMAP_HELVETICA_18);
        drawText(WINDOW_WIDTH / 2 - 130, WINDOW_HEIGHT / 2 - 20, "PRESS SPACE TO RESTART", GLUT_BITMAP_HELVETICA_18);
    }

    glutSwapBuffers();
}

// Keyboard Inputs
void keyboard(unsigned char key, int x, int y) {
    if (key == ' ') { // Spacebar
        if (currentState == START) {
            currentState = PLAYING;
            velocity = jumpStrength;
        } else if (currentState == PLAYING) {
            velocity = jumpStrength; // Jump
        } else if (currentState == GAME_OVER) {
            resetGame(); // Restart game
        }
    }
}

// Main Function
int main(int argc, char** argv) {
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Flappy Bird GLUT Project");

    // 2D Orthographic Projection Setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
