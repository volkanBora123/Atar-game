#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define SCREEN_WIDTH  40
#define SCREEN_HEIGHT 21
#define LOG_SYMBOL '|'
#define PLAYER_SYMBOL 'O'
#define RIVER_SYMBOL '~'
int DELAY = 100000; // Microseconds

struct termios orig_termios;

// Player position
typedef struct {
    int x, y;
} Player;

// Log structure
typedef struct {
    int x, y;
    int active; // 1 if log is on screen, 0 otherwise
} Log;

// Global variables
Player player;
Log logs[SCREEN_HEIGHT];
int score = 0;

// Function prototypes
void clearScreen();
void disableRawMode();
void enableRawMode();
void enableNonBlockingInput();
void disableNonBlockingInput();
void handleExit(int sig);
void initGame();
void drawGame();
void updateLogs();
void checkCollision();
void movePlayer(char input);

// Restore terminal settings after exiting the program
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Configure the terminal for real-time input (raw mode)
void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Enable non-blocking input
void enableNonBlockingInput() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

// Disable non-blocking input
void disableNonBlockingInput() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK); // Clear non-blocking flag
}

// Signal handler to gracefully exit on termination signals
void handleExit(int sig) {
    disableRawMode();
    disableNonBlockingInput();
    clearScreen();
    exit(0);
}

// Clear the screen using ANSI escape codes
void clearScreen() {
    printf("\033[H\033[J");
}

// Initialize the game state
void initGame() {
    player.x = SCREEN_WIDTH / 2;
    player.y = SCREEN_HEIGHT - 1;

    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        logs[i].active = 0;
    }

    srand(time(0)); // Seed random number generator
}

// Draw the game state
void drawGame() {
    clearScreen();

    // Draw the top info bar
    printf("Use 'w', 'a', 's', 'd' to move. Score: %d\n", score);

    // Draw the game grid
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (y == player.y && x == player.x) {
                printf("%c", PLAYER_SYMBOL); // Draw player
            } else {
                int isLog = 0;
                for (int i = 0; i < SCREEN_HEIGHT; i++) {
                    if (logs[i].active && logs[i].x == x && logs[i].y == y) {
                        printf("%c", LOG_SYMBOL); // Draw log
                        isLog = 1;
                        break;
                    }
                }
                if (!isLog) {
                    if (y == SCREEN_HEIGHT - 1 || y == 0) {
                        printf("%c", RIVER_SYMBOL); // Draw river
                    } else {
                        printf(" "); // Empty space
                    }
                }
            }
        }
        printf("\n");
    }
}

// Update log positions
void updateLogs() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        if (logs[i].active) {
            logs[i].x += 1; // Move log to the right
            if (logs[i].x >= SCREEN_WIDTH) {
                logs[i].active = 0; // Deactivate log if it goes out of bounds
            }
        } else if (rand() % 20 == 0 && i != SCREEN_HEIGHT - 1 && i != 0) {
            // Prevent logs from spawning on the top and bottom lines
            logs[i].active = 1;
            logs[i].x = 0; // Spawn log at the left edge
            logs[i].y = i;
        }
    }
}

// Check for collisions
void checkCollision() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        if (logs[i].active && logs[i].x == player.x && logs[i].y == player.y) {
            // Collision detected
            score = 0;                // Reset the score
            player.x = SCREEN_WIDTH / 2; // Keep player centered horizontally
            player.y = SCREEN_HEIGHT - 1; // Respawn player at the starting row
            // Clear all logs
            for (int j = 0; j < SCREEN_HEIGHT; j++) {
                logs[j].active = 0;
            }
            break;
        }
    }
}

// Move player
void movePlayer(char input) {
    switch (input) {
        case 'a': // Move left
            if (player.x > 0) player.x--;
            break;
        case 'd': // Move right
            if (player.x < SCREEN_WIDTH - 1) player.x++;
            break;
        case 'w': // Move up
            if (player.y >= 1) {
                player.y--;
                score += 1; // Increment score
            }
            if (player.y == 0) {
                player.y = SCREEN_HEIGHT - 1;
                score += 100;
            }
            break;
        case 's': // Move down
            if (player.y < SCREEN_HEIGHT - 1) {
                player.y++;
                score--;
            }
            break;
        case 'q': // Exit game
            disableRawMode();
            disableNonBlockingInput(); // Restore input mode
            clearScreen(); // Ensure clean exit
            exit(0);
    }
}

int main() {
    char input;

    // Set up signal handling
    signal(SIGINT, handleExit);
    signal(SIGHUP, handleExit);  // Handle terminal disconnect
    signal(SIGTERM, handleExit); // Handle termination signal
    signal(SIGQUIT, handleExit); // Handle quit signal

    enableRawMode();
    enableNonBlockingInput();

    initGame();

    while (1) {
        if (read(STDIN_FILENO, &input, 1) == 1) {
            movePlayer(input);
        }

        updateLogs();
        checkCollision();
        drawGame();
        usleep(DELAY); // Control game speed
    }

    // Cleanup (though this won't be reached due to the infinite loop)
    disableRawMode();
    disableNonBlockingInput();
    return 0;
}

