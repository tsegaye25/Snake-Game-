#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <windows.h>
#include <mmsystem.h>

// Game constants
const int GRID_WIDTH = 40;
const int GRID_HEIGHT = 30;
const int CELL_SIZE = 15;
const int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;
const int GAME_SPEED = 100; // milliseconds between updates
const int MAX_HISTORY = 10; // maximum number of high scores to keep

// Directions
enum Direction { UP, DOWN, LEFT, RIGHT };

// Game states
enum GameState { MENU, PLAYING, GAME_OVER };

// Point structure
struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// Score record structure
struct ScoreRecord {
    int score;
    std::string date;

    ScoreRecord(int s = 0, const std::string& d = "") : score(s), date(d) {}

    bool operator<(const ScoreRecord& other) const {
        return score > other.score; // Sort in descending order
    }
};

// Game variables
std::vector<Point> snake;
Point food;
Direction direction = RIGHT;
Direction nextDirection = RIGHT;
GameState gameState = MENU;
int score = 0;
bool foodEaten = false;
std::vector<ScoreRecord> scoreHistory;
std::string playerName = "Player";

// Function prototypes
void initGame();
void updateGame();
void drawScene();
void drawSnake();
void drawFood();
void drawGrid();
void drawScore();
void drawMenu();
void drawGameOver();
void spawnFood();
bool checkCollision();
void keyboardFunc(unsigned char key, int x, int y);
void specialKeyFunc(int key, int x, int y);
void timerFunc(int value);
void reshapeFunc(int width, int height);
void loadScoreHistory();
void saveScoreHistory();
void addScore(int score);
std::string getCurrentDate();
void playSound(const char* soundFile);

// Initialize the game
void initGame() {
    // Clear the snake
    snake.clear();

    // Create the snake with 3 segments in the middle of the grid
    int startX = GRID_WIDTH / 2;
    int startY = GRID_HEIGHT / 2;
    snake.push_back(Point(startX, startY));       // Head
    snake.push_back(Point(startX - 1, startY));   // Body
    snake.push_back(Point(startX - 2, startY));   // Tail

    // Initialize direction
    direction = RIGHT;
    nextDirection = RIGHT;

    // Spawn initial food
    spawnFood();

    // Reset score
    score = 0;

    // Set game state to playing
    gameState = PLAYING;

    // Load score history
    loadScoreHistory();
}

// Update game state
void updateGame() {
    if (gameState != PLAYING) return;

    // Update direction
    direction = nextDirection;

    // Get the current head position
    Point head = snake.front();

    // Calculate new head position based on direction
    switch (direction) {
        case UP:
            head.y = (head.y + 1) % GRID_HEIGHT;
            break;
        case DOWN:
            head.y = (head.y - 1 + GRID_HEIGHT) % GRID_HEIGHT;
            break;
        case LEFT:
            head.x = (head.x - 1 + GRID_WIDTH) % GRID_WIDTH;
            break;
        case RIGHT:
            head.x = (head.x + 1) % GRID_WIDTH;
            break;
    }

    // Check if the snake hit itself
    for (size_t i = 0; i < snake.size(); i++) {
        if (head == snake[i]) {
            gameState = GAME_OVER;
            playSound("gameover.wav");
            addScore(score);
            saveScoreHistory();
            return;
        }
    }

    // Add new head to the snake
    snake.insert(snake.begin(), head);

    // Check if food was eaten
    if (head == food) {
        // Increase score
        score += 10;

        // Spawn new food
        spawnFood();

        // Play sound
        playSound("eat.wav");

        // Don't remove the tail (snake grows)
        foodEaten = true;
    } else {
        // Remove the tail (snake moves)
        snake.pop_back();
        foodEaten = false;
    }
}

// Draw the scene
void drawScene() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw based on game state
    switch (gameState) {
        case MENU:
            drawMenu();
            break;
        case PLAYING:
            drawGrid();
            drawSnake();
            drawFood();
            drawScore();
            break;
        case GAME_OVER:
            drawGrid();
            drawSnake();
            drawFood();
            drawScore();
            drawGameOver();
            break;
    }

    // Swap buffers
    glutSwapBuffers();
}

// Draw the snake
void drawSnake() {
    // Draw snake body
    glColor3f(0.0f, 0.7f, 0.0f); // Green
    for (size_t i = 1; i < snake.size(); i++) {
        glBegin(GL_QUADS);
        glVertex2f(snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE);
        glVertex2f((snake[i].x + 1) * CELL_SIZE, snake[i].y * CELL_SIZE);
        glVertex2f((snake[i].x + 1) * CELL_SIZE, (snake[i].y + 1) * CELL_SIZE);
        glVertex2f(snake[i].x * CELL_SIZE, (snake[i].y + 1) * CELL_SIZE);
        glEnd();
    }

    // Draw snake head
    glColor3f(0.0f, 0.9f, 0.0f); // Brighter green
    glBegin(GL_QUADS);
    glVertex2f(snake[0].x * CELL_SIZE, snake[0].y * CELL_SIZE);
    glVertex2f((snake[0].x + 1) * CELL_SIZE, snake[0].y * CELL_SIZE);
    glVertex2f((snake[0].x + 1) * CELL_SIZE, (snake[0].y + 1) * CELL_SIZE);
    glVertex2f(snake[0].x * CELL_SIZE, (snake[0].y + 1) * CELL_SIZE);
    glEnd();

    // Draw eyes
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    float eyeSize = CELL_SIZE / 5.0f;
    float eyeOffset = CELL_SIZE / 4.0f;

    // Left eye
    glBegin(GL_QUADS);
    glVertex2f(snake[0].x * CELL_SIZE + eyeOffset, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize);
    glVertex2f(snake[0].x * CELL_SIZE + eyeOffset + eyeSize, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize);
    glVertex2f(snake[0].x * CELL_SIZE + eyeOffset + eyeSize, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset);
    glVertex2f(snake[0].x * CELL_SIZE + eyeOffset, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset);
    glEnd();

    // Right eye
    glBegin(GL_QUADS);
    glVertex2f(snake[0].x * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize);
    glVertex2f(snake[0].x * CELL_SIZE + CELL_SIZE - eyeOffset, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize);
    glVertex2f(snake[0].x * CELL_SIZE + CELL_SIZE - eyeOffset, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset);
    glVertex2f(snake[0].x * CELL_SIZE + CELL_SIZE - eyeOffset - eyeSize, snake[0].y * CELL_SIZE + CELL_SIZE - eyeOffset);
    glEnd();
}

// Draw the food
void drawFood() {
    glColor3f(1.0f, 0.0f, 0.0f); // Red

    // Draw food as a circle
    glBegin(GL_TRIANGLE_FAN);
    float centerX = food.x * CELL_SIZE + CELL_SIZE / 2.0f;
    float centerY = food.y * CELL_SIZE + CELL_SIZE / 2.0f;
    float radius = CELL_SIZE / 2.0f;

    glVertex2f(centerX, centerY); // Center

    for (int i = 0; i <= 360; i += 10) {
        float angle = i * 3.14159f / 180.0f;
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        glVertex2f(x, y);
    }

    glEnd();
}

// Draw the grid
void drawGrid() {
    glColor3f(0.2f, 0.2f, 0.2f); // Dark gray
    glBegin(GL_LINES);

    // Vertical lines
    for (int i = 0; i <= GRID_WIDTH; i++) {
        glVertex2f(i * CELL_SIZE, 0);
        glVertex2f(i * CELL_SIZE, WINDOW_HEIGHT);
    }

    // Horizontal lines
    for (int i = 0; i <= GRID_HEIGHT; i++) {
        glVertex2f(0, i * CELL_SIZE);
        glVertex2f(WINDOW_WIDTH, i * CELL_SIZE);
    }

    glEnd();
}

// Draw the score
void drawScore() {
    // Set color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Set position for the score text
    glRasterPos2f(10, WINDOW_HEIGHT - 20);

    // Convert score to string
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);

    // Draw each character
    for (int i = 0; scoreText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)scoreText[i]);
    }
}

// Draw the menu
void drawMenu() {
    // Set color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Title
    const char* title = "SNAKE GAME";
    glRasterPos2f(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 100);
    for (int i = 0; title[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)title[i]);
    }

    // Instructions
    const char* instructions = "Press SPACE to start";
    glRasterPos2f(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2);
    for (int i = 0; instructions[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)instructions[i]);
    }

    // Controls
    const char* controls1 = "Use arrow keys or WASD to move";
    glRasterPos2f(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 30);
    for (int i = 0; controls1[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (int)controls1[i]);
    }

    // High scores
    glRasterPos2f(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 80);
    const char* highScoreTitle = "HIGH SCORES";
    for (int i = 0; highScoreTitle[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)highScoreTitle[i]);
    }

    // Display high scores
    int y = WINDOW_HEIGHT / 2 - 110;
    for (size_t i = 0; i < scoreHistory.size() && i < 5; i++) {
        char scoreEntry[100];
        sprintf(scoreEntry, "%d. %d - %s", (int)(i + 1), scoreHistory[i].score, scoreHistory[i].date.c_str());
        glRasterPos2f(WINDOW_WIDTH / 2 - 100, y);
        for (int j = 0; scoreEntry[j] != '\0'; j++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (int)scoreEntry[j]);
        }
        y -= 20;
    }
}

// Draw game over screen
void drawGameOver() {
    // Semi-transparent overlay
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // Game over text
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    const char* gameOverText = "GAME OVER";
    glRasterPos2f(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 30);
    for (int i = 0; gameOverText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)gameOverText[i]);
    }

    // Final score
    glColor3f(1.0f, 1.0f, 1.0f); // White
    char finalScoreText[50];
    sprintf(finalScoreText, "Final Score: %d", score);
    glRasterPos2f(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2);
    for (int i = 0; finalScoreText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)finalScoreText[i]);
    }

    // Restart instructions
    const char* restartText = "Press SPACE to play again";
    glRasterPos2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 30);
    for (int i = 0; restartText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)restartText[i]);
    }

    // Menu instructions
    const char* menuText = "Press M for menu";
    glRasterPos2f(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 60);
    for (int i = 0; menuText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)menuText[i]);
    }
}

// Spawn food at a random location
void spawnFood() {
    // Create a list of available positions
    std::vector<Point> availablePositions;

    // Add all grid positions to the list
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            Point p(x, y);

            // Check if the position is occupied by the snake
            bool occupied = false;
            for (size_t j = 0; j < snake.size(); j++) {
                const Point& segment = snake[j];
                if (p == segment) {
                    occupied = true;
                    break;
                }
            }

            // If not occupied, add to available positions
            if (!occupied) {
                availablePositions.push_back(p);
            }
        }
    }

    // If there are available positions, choose one randomly
    if (!availablePositions.empty()) {
        int index = rand() % availablePositions.size();
        food = availablePositions[index];
    }
}

// Keyboard function for regular keys
void keyboardFunc(unsigned char key, int x, int y) {
    switch (key) {
        case ' ': // Space
            if (gameState == MENU) {
                initGame();
            } else if (gameState == GAME_OVER) {
                initGame();
            }
            break;
        case 'm': // M
        case 'M': // M
            if (gameState == GAME_OVER) {
                gameState = MENU;
            }
            break;
        case 'w': // W
        case 'W': // W
            if (direction != DOWN && gameState == PLAYING) {
                nextDirection = UP;
            }
            break;
        case 's': // S
        case 'S': // S
            if (direction != UP && gameState == PLAYING) {
                nextDirection = DOWN;
            }
            break;
        case 'a': // A
        case 'A': // A
            if (direction != RIGHT && gameState == PLAYING) {
                nextDirection = LEFT;
            }
            break;
        case 'd': // D
        case 'D': // D
            if (direction != LEFT && gameState == PLAYING) {
                nextDirection = RIGHT;
            }
            break;
        case 27: // Escape
            exit(0);
            break;
    }

    glutPostRedisplay();
}

// Special keyboard function for arrow keys
void specialKeyFunc(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            if (direction != DOWN && gameState == PLAYING) {
                nextDirection = UP;
            }
            break;
        case GLUT_KEY_DOWN:
            if (direction != UP && gameState == PLAYING) {
                nextDirection = DOWN;
            }
            break;
        case GLUT_KEY_LEFT:
            if (direction != RIGHT && gameState == PLAYING) {
                nextDirection = LEFT;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (direction != LEFT && gameState == PLAYING) {
                nextDirection = RIGHT;
            }
            break;
    }

    glutPostRedisplay();
}

// Timer function for game updates
void timerFunc(int value) {
    // Update game state
    if (gameState == PLAYING) {
        updateGame();
    }

    // Redraw the scene
    glutPostRedisplay();

    // Set the next timer
    glutTimerFunc(GAME_SPEED, timerFunc, 0);
}

// Reshape function
void reshapeFunc(int width, int height) {
    // Set the viewport
    glViewport(0, 0, width, height);

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
}

// Load score history from file
void loadScoreHistory() {
    scoreHistory.clear();

    std::ifstream file("scores.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            int score;
            std::string date;

            if (iss >> score) {
                // Get the rest of the line as the date
                std::getline(iss >> std::ws, date);

                // Add to history
                scoreHistory.push_back(ScoreRecord(score, date));
            }
        }
        file.close();

        // Sort the history
        std::sort(scoreHistory.begin(), scoreHistory.end());
    }
}

// Save score history to file
void saveScoreHistory() {
    std::ofstream file("scores.txt");
    if (file.is_open()) {
        for (size_t i = 0; i < scoreHistory.size(); i++) {
            const ScoreRecord& record = scoreHistory[i];
            file << record.score << " " << record.date << std::endl;
        }
        file.close();
    }
}

// Add a new score to history
void addScore(int score) {
    // Create a new record
    ScoreRecord record(score, getCurrentDate());

    // Add to history
    scoreHistory.push_back(record);

    // Sort the history
    std::sort(scoreHistory.begin(), scoreHistory.end());

    // Trim if necessary
    if (scoreHistory.size() > MAX_HISTORY) {
        scoreHistory.resize(MAX_HISTORY);
    }
}

// Get current date as string
std::string getCurrentDate() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);

    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d", timeinfo);

    return std::string(buffer);
}

// Play a sound
void playSound(const char* soundFile) {
    PlaySound(soundFile, NULL, SND_ASYNC | SND_FILENAME);
}

// Display function
void displayFunc() {
    drawScene();
}

// Main function
int main(int argc, char** argv) {
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Snake Game");

    // Set up callbacks
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutKeyboardFunc(keyboardFunc);
    glutSpecialFunc(specialKeyFunc);
    glutTimerFunc(GAME_SPEED, timerFunc, 0);

    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load score history
    loadScoreHistory();

    // Start the main loop
    glutMainLoop();

    return 0;
}
