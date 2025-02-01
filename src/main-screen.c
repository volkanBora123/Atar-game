#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_GAMES 100

// Forward declarations
void restore_canonical_mode();
void set_non_canonical_mode();
void display_main_menu(int selected, int game_count, int game_selected, char *formatted_names[]);
void launch_game(const char *game);
int get_game_files(char *games[], char *formatted_names[], int max_games);

// Original terminal settings
struct termios original_tio;

// Global state variables
pid_t child_pid = -1; // Stores the child process PID if a game is running

// Signal handler for the parent process
void parent_signal_handler(int sig) {
    if (sig == SIGINT) {
        // If a game is running, terminate it and return to the menu
        if (child_pid > 0) {
            kill(child_pid, SIGINT); // Send SIGINT to the child process
            waitpid(child_pid, NULL, 0); // Wait for the child process to terminate
            child_pid = -1; // Reset child_pid
        } else {
          restore_canonical_mode();
                  printf("\033[H\033[J");

                  exit(0);
        }
    } else if (sig == SIGTERM) {
        // If SIGTERM is received, terminate both the menu and the game
        if (child_pid > 0) {
            kill(child_pid, SIGTERM); // Ensure the child process is terminated
            waitpid(child_pid, NULL, 0);
        }
        restore_canonical_mode();
        printf("\033[H\033[J");
        child_pid = -1; // Reset child_pid
        exit(0);
    }
}

// Signal handler for the child process (game)
void child_signal_handler(int sig) {
    if (sig == SIGINT) {
        // Ignore SIGINT (Ctrl+C) to avoid terminating the game
        printf("\nGame interrupted. Returning to main menu...\n");
        exit(0);
    }
    // Ignore SIGTERM as it is meant for parent termination
}

// Enable non-canonical mode
void set_non_canonical_mode() {
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &original_tio);
    new_tio = original_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO); // Disable line buffering and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

// Restore terminal to canonical mode
void restore_canonical_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

// Get game files in the current directory
int get_game_files(char *games[], char *formatted_names[], int max_games) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    if ((dir = opendir(".")) == NULL) {
        perror("Failed to open directory");
        return 0;
    }

    // Scan for files starting with "game_" that are executable
    while ((entry = readdir(dir)) != NULL) {
        if (count >= max_games) break;

        if (strncmp(entry->d_name, "game_", 5) == 0 && access(entry->d_name, X_OK) == 0) {
            games[count] = strdup(entry->d_name);

            // Format the name: remove "game_" and capitalize the first letter
            char *name = entry->d_name + 5;
            char *formatted_name = strdup(name);
            formatted_name[0] = toupper(formatted_name[0]);

            formatted_names[count] = formatted_name;
            count++;
        }
    }

    closedir(dir);
    return count;
}

// Display the main menu
void display_main_menu(int selected, int game_count, int game_selected, char *formatted_names[]) {
    printf("\033[H\033[J"); // Clear the screen
    printf("###################################################\n");
    printf("#          WELCOME TO ATARI CONSOLE               #\n");
    printf("#             Created by Volkan                   #\n");
    printf("#                                                 #\n");
    printf("#   Use 'a' and 'd' to navigate                   #\n");
    printf("#   Use 'w' and 's' to switch game at games       #\n");
    printf("#   Press 'enter' to play at play                 #\n");
    printf("#   Press 'q' to exit                             #\n");
    printf("###################################################\n\n");

    if (selected == 0) {
        printf("-> [(Play)] <-       ");
    } else {
        printf("    Play           ");
    }

    if (selected == 1) {
        printf("-> [( %s )] <-", formatted_names[game_selected]);
    } else {
        printf("    %s     ", formatted_names[game_selected]);
    }

    if (selected == 2) {
        printf("       -> [(Exit)] <-\n");
    } else {
        printf("           Exit\n");
    }
}

// Launch a game
void launch_game(const char *game) {
    printf("\nLaunching game: %s\n", game);
    fflush(stdout);

    child_pid = fork();

    if (child_pid == 0) {
        // Child process: run the game
        signal(SIGINT, child_signal_handler); // Handle SIGINT for the game
        signal(SIGTERM, SIG_IGN); // Ignore SIGTERM in the game process

        char path[256];
        snprintf(path, sizeof(path), "./%s", game);
        execlp(path, game, NULL);
        perror("Error launching game");
        exit(EXIT_FAILURE);
    } else if (child_pid > 0) {
        // Parent process: wait for the game to finish
        int status;
        waitpid(child_pid, &status, 0);
        child_pid = -1; // Reset child_pid after the game exits
        printf("\nGame exited. Returning to main menu...\n");
        sleep(2);
    } else {
        perror("Error forking process");
    }
}

int main() {
    // Set up signal handlers for the parent process
    signal(SIGINT, parent_signal_handler);
    signal(SIGTERM, parent_signal_handler);

    set_non_canonical_mode();

    char *games[MAX_GAMES] = {NULL};
    char *formatted_names[MAX_GAMES] = {NULL};
    int game_count = get_game_files(games, formatted_names, MAX_GAMES);
    int selected = 0;
    int game_selected = 0;
    int quit_program = 0;

    // Handle case when no games are found
    if (game_count == 0) {
        printf("No games found! Exiting...\n");
        restore_canonical_mode();
        return 1;
    }

    while (!quit_program) {
        display_main_menu(selected, game_count, game_selected, formatted_names);
        char input = getchar();

        if (input == 'q') {
            quit_program = 1; // Exit on 'q'
        } else if (input == 'a') {
            selected = (selected - 1 + 3) % 3; // Navigate left
        } else if (input == 'd') {
            selected = (selected + 1) % 3; // Navigate right
        } else if (input == 'w' && selected == 1) {
            // Move selection up in the game list
            game_selected = (game_selected - 1 + game_count) % game_count;
        } else if (input == 's' && selected == 1) {
            // Move selection down in the game list
            game_selected = (game_selected + 1) % game_count;
        } else if (input == '\n') {
            if (selected == 0) {
                // Play button
                launch_game(games[game_selected]);
            } else if (selected == 2) {
                // Exit
                quit_program = 1;
            }
        }
    }

    for (int i = 0; i < game_count; i++) {
        if (games[i]) free(games[i]);
        if (formatted_names[i]) free(formatted_names[i]);
    }

    restore_canonical_mode();

    // Clear the terminal before final exit
    printf("\033[H\033[J");
    return 0;
}

