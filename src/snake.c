#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

// Game configuration
#define GRID_SIZE 15
#define EMPTY_CELL '.'
#define SNAKE_HEAD 'O'
#define SNAKE_BODY '#'
#define BAIT 'X'

// Global game state
char grid[GRID_SIZE][GRID_SIZE];
int *snake_x, *snake_y;   // Dynamic arrays to track the snake's body positions
int snake_length;         // Length of the snake
int bait_x, bait_y;       // Coordinates of the bait
char direction = 'd';     // Initial direction: right
int score = 0;            // Player's score
int running = 1;          // Game running state
int paused = 0;           // Pause state flag

// Terminal settings
struct termios orig_termios;

// Function declarations
void init_game();
void spawn_bait();
void update_snake();
void draw_grid();
int move_snake();
int check_collision(int x, int y);
void reset_terminal();
void handle_signal(int sig);
void setup_terminal();
int kbhit();
char getch();

int main() {
    // Prepare the terminal for real-time input
    setup_terminal();
    signal(SIGINT, handle_signal);  // Ensure Ctrl+C exits gracefully
    signal(SIGTERM, handle_signal); // Handle termination signals cleanly

    // Initialize game state
    init_game();

    // Main game loop
    while (running) {
        draw_grid(); // Display the game grid

        // Check for player input to change direction
        if (kbhit()) {
            char input = getch();
            if (input == 'q') {  // Exit game on 'q'
                running = 0;
                break;
            }
            // Update direction if it doesn't reverse the snake
            if ((input == 'w' && direction != 's') ||
                (input == 'a' && direction != 'd') ||
                (input == 's' && direction != 'w') ||
                (input == 'd' && direction != 'a')) {
                direction = input;
                paused = 0; // Resume if paused
            }

            // Clear the input buffer after processing the key
            tcflush(STDIN_FILENO, TCIFLUSH);
        }

        // Only move the snake if not paused
        if (!paused) {
            if (!move_snake()) {
                paused = 1; // Pause if a collision occurs
            }
        }

        usleep(100000); // Slow down the loop for playable snake speed
    }

    // Clean up resources and restore terminal
    reset_terminal();
    free(snake_x);
    free(snake_y);
    return 0;
}

// Set up the initial game state
void init_game() {
    // Clear the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = EMPTY_CELL;
        }
    }

    // Initialize the snake in the middle of the grid
    snake_length = 2;
    snake_x = malloc(snake_length * sizeof(int));
    snake_y = malloc(snake_length * sizeof(int));
    snake_x[0] = GRID_SIZE / 2; // Head
    snake_y[0] = GRID_SIZE / 2;
    snake_x[1] = GRID_SIZE / 2; // Tail
    snake_y[1] = GRID_SIZE / 2 - 1;

    // Place the snake on the grid
    update_snake();

    // Add the first bait
    spawn_bait();
}

// Place a bait at a random empty position
void spawn_bait() {
    do {
        bait_x = rand() % GRID_SIZE;
        bait_y = rand() % GRID_SIZE;
    } while (grid[bait_x][bait_y] != EMPTY_CELL); // Retry if spot is occupied
    grid[bait_x][bait_y] = BAIT;
}

// Update the grid with the snake's current position
void update_snake() {
    // Reset the grid, but keep the bait
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] != BAIT) {
                grid[i][j] = EMPTY_CELL;
            }
        }
    }

    // Add the snake to the grid
    for (int i = 0; i < snake_length; i++) {
        if (i == 0) {
            grid[snake_x[i]][snake_y[i]] = SNAKE_HEAD;
        } else {
            grid[snake_x[i]][snake_y[i]] = SNAKE_BODY;
        }
    }
}

// Render the game grid and score
void draw_grid() {
    printf("\033[H\033[J"); // Clear the screen
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            putchar(grid[i][j]);
            printf(" ");
        }
        putchar('\n');
    }
    printf("Score: %d\n", score);
    printf("Use 'w', 'a', 's', 'd' to move. Press 'q' to quit.\n");
    if (paused) {
        printf("Game paused! Press a direction to resume.\n");
    }
}

// Move the snake in the current direction
int move_snake() {
    int new_head_x = snake_x[0];
    int new_head_y = snake_y[0];

    // Update the head position based on direction
    switch (direction) {
        case 'w': new_head_x--; break;
        case 'a': new_head_y--; break;
        case 's': new_head_x++; break;
        case 'd': new_head_y++; break;
    }

    // Check for collisions
    if (check_collision(new_head_x, new_head_y)) {
        return 0; // Collision means game pause
    }

    // Check if the snake eats the bait
    int grow = (new_head_x == bait_x && new_head_y == bait_y);

    if (!grow) {
        // If not growing, clear the last tail position
        grid[snake_x[snake_length - 1]][snake_y[snake_length - 1]] = EMPTY_CELL;
    } else {
        // Extend the snake
        score++;
        snake_length++;
        snake_x = realloc(snake_x, snake_length * sizeof(int));
        snake_y = realloc(snake_y, snake_length * sizeof(int));
        spawn_bait();
    }

    // Move the snake's body
    for (int i = snake_length - 1; i > 0; i--) {
        snake_x[i] = snake_x[i - 1];
        snake_y[i] = snake_y[i - 1];
    }

    // Update the head
    snake_x[0] = new_head_x;
    snake_y[0] = new_head_y;

    update_snake();
    return 1;
}

// Check for collisions with the wall or the snake's body
int check_collision(int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) {
        return 1; // Wall collision
    }
    for (int i = 0; i < snake_length; i++) {
        if (snake_x[i] == x && snake_y[i] == y) {
            return 1; // Self-collision
        }
    }
    return 0;
}

// Configure the terminal for non-blocking input
void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable line buffering and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

// Restore the original terminal settings
void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Handle signals like Ctrl+C
void handle_signal(int sig) {
    reset_terminal();
    printf("\nGame over! Final score: %d\n", score);
    exit(0);
}

// Check if a key has been pressed
int kbhit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

// Get a single character from input
char getch() {
    char buf = 0;
    read(STDIN_FILENO, &buf, 1);
    return buf;
}

