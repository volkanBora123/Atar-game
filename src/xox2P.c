#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#define ROWS 3
#define COLS 3

char board[ROWS][COLS];
int cursor_row = 0, cursor_col = 0;
char current_player = 'X';
struct termios oldt; // Store original terminal settings

// Signal handler to clean up and exit gracefully
void signal_handler(int signum) {
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    exit(0);
}

// Function to display the game board with the cursor position highlighted
void display_board() {
    system("clear");
    printf("Player %c's turn\n", current_player);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == cursor_row && j == cursor_col) {
                printf("[");
                if (board[i][j] == ' ') printf(" ");
                else printf("%c", board[i][j]);
                printf("]");
            } else {
                printf(" %c ", board[i][j]);
            }
            if (j < COLS - 1) printf("|");
        }
        printf("\n");
        if (i < ROWS - 1) {
            for (int j = 0; j < COLS; j++) {
                printf("---");
                if (j < COLS - 1) printf("|");
            }
            printf("\n");
        }
    }
    printf("\n");
}

// Function to check if the game is over
char check_winner() {
    for (int i = 0; i < ROWS; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') {
            return board[i][0];
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ') {
            return board[0][i];
        }
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') {
        return board[0][0];
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') {
        return board[0][2];
    }
    int empty = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == ' ') empty++;
        }
    }
    return (empty == 0) ? 'T' : ' ';
}

void set_raw_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restore_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void init_board() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = ' ';
        }
    }
    cursor_row = 0;
    cursor_col = 0;
    current_player = 'X';
}

void clear_input_buffer() {
    tcflush(STDIN_FILENO, TCIFLUSH);
}

int prompt_restart() {
    char choice;
    printf("Do you want to play again? (y/n): ");
    while (1) {
        choice = getchar();
        if (choice == 'y' || choice == 'Y') {
            return 1;
        } else if (choice == 'n' || choice == 'N' || choice == 'q' || choice == 'Q') {
            restore_mode();
            printf("\nGame exited. Thanks for playing!\n");
            return 0;
        }
    }
}

int main() {
    // Register signal handlers
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    set_raw_mode();

    while (1) {
        init_board();
        char winner = ' ';
        while (1) {
            display_board();
            char input = getchar();
            if (input == 'q') {
                clear_input_buffer();
                restore_mode();
                printf("Game exited. Thanks for playing!\n");
                return 0;
            }

            switch (input) {
                case 'w': cursor_row = (cursor_row > 0) ? cursor_row - 1 : cursor_row; break;
                case 's': cursor_row = (cursor_row < ROWS - 1) ? cursor_row + 1 : cursor_row; break;
                case 'a': cursor_col = (cursor_col > 0) ? cursor_col - 1 : cursor_col; break;
                case 'd': cursor_col = (cursor_col < COLS - 1) ? cursor_col + 1 : cursor_col; break;
                case '\n': 
                    if (board[cursor_row][cursor_col] == ' ') {
                        board[cursor_row][cursor_col] = current_player;
                        current_player = (current_player == 'X') ? 'O' : 'X';
                    }
                    break;
            }
            winner = check_winner();

            if (winner != ' ') {
                display_board();
                if (winner == 'X' || winner == 'O') {
                    printf("Player %c wins!\n", winner);
                } else if (winner == 'T') {
                    printf("It's a tie!\n");
                }
                break;
            }
        }

        if (!prompt_restart()) {
            break;
        }
    }

    restore_mode();
    return 0;
}

