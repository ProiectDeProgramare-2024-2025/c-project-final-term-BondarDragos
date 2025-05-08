#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_QUESTIONS 50
#define MAX_LINE 256
#define NAME_LENGTH 50
#define MAX_PLAYERS 100
#define COLOR_RED    "\033[1;31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE   "\033[1;34m"
#define COLOR_RESET  "\033[0m"
typedef struct {
    char question[MAX_LINE];
    char options[4][MAX_LINE];
    char correct;
    int difficulty;
} Question;

typedef struct {
    char name[NAME_LENGTH];
    int score;
} PlayerScore;


int loadQuestions(Question questions[]) {
    FILE *f = fopen("questions.txt", "r");
    if (!f) {
        printf("Could not open questions.txt\n");
        return 0;
    }

    int count = 0;
    while (count < MAX_QUESTIONS && fgets(questions[count].question, MAX_LINE, f)) {
        for (int i = 0; i < 4; i++)
            fgets(questions[count].options[i], MAX_LINE, f);

        char correctLine[MAX_LINE];
        fgets(correctLine, MAX_LINE, f);
        questions[count].correct = toupper(correctLine[0]);

        char diffLine[MAX_LINE];
        fgets(diffLine, MAX_LINE, f);
        questions[count].difficulty = atoi(diffLine);

        fgets(diffLine, MAX_LINE, f); // skip empty line
        count++;
    }

    fclose(f);
    return count;
}

void saveGameState(int selected[], int answered, int score, const char* username) {
    FILE* f = fopen("saved_game.txt", "w");
    if (!f) return;

    for (int i = 0; i < 10; i++)
        fprintf(f, "%d ", selected[i]);
    fprintf(f, "\n%d\n%d\n%s\n", answered, score, username);

    fclose(f);
}

void saveToLeaderboard(const char* playerName, int score) {
    FILE* file = fopen("leaderboard.txt", "a");
    if (file == NULL) {
        printf("Error opening leaderboard file.\n");
        return;
    }
    fprintf(file, "%s %d\n", playerName, score);
    fclose(file);
}

int compareScores(const void* a, const void* b) {
    PlayerScore* p1 = (PlayerScore*)a;
    PlayerScore* p2 = (PlayerScore*)b;
    return p2->score - p1->score; // Descending
}

void showLeaderboard() {
    FILE* file = fopen("leaderboard.txt", "r");
    if (file == NULL) {
        printf("No leaderboard available yet.\n");
        return;
    }

    PlayerScore players[MAX_PLAYERS];
    int count = 0;
    
    while (fscanf(file, "%s %d", players[count].name, &players[count].score) == 2) {
        count++;
        if (count >= MAX_PLAYERS) break;
    }
    

    qsort(players, count, sizeof(PlayerScore), compareScores);

    
    printf("\n====== Leaderboard ======\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s - %d\n", i + 1, players[i].name, players[i].score);
    }
    printf("=========================\n");
    fclose(file);

}

void playGame(Question questions[], int totalQuestions) {
    int selected[10], score = 0;
    char name[NAME_LENGTH];

    srand(time(NULL));
    for (int i = 0; i < 10; ) {
        int r = rand() % totalQuestions;
        int unique = 1;
        for (int j = 0; j < i; j++)
            if (selected[j] == r) unique = 0;
        if (unique) selected[i++] = r;
    }

    printf("Enter your name: ");
    scanf("%s", name);

    for (int i = 0; i < 10; i++) {
        int idx = selected[i];
        printf("\nQuestion %d:\n%s", i + 1, questions[idx].question);
        printf("A. " COLOR_GREEN "%s" COLOR_RESET, questions[idx].options[0]);
        printf("B. " COLOR_YELLOW "%s" COLOR_RESET, questions[idx].options[1]);
        printf("C. " COLOR_BLUE "%s" COLOR_RESET, questions[idx].options[2]);
        printf("D. " COLOR_RED "%s" COLOR_RESET, questions[idx].options[3]);

        printf("Enter your answer (A/B/C/D) or X to save & exit: ");
        char answer;
        scanf(" %c", &answer);
        answer = toupper(answer);

        if (answer == 'X') {
            saveGameState(selected, i, score, name);
            printf("Game progress saved! Returning to main menu.\n");
            return;
        }

        if (answer == questions[idx].correct) {
            int pts = questions[idx].difficulty;
            score += pts;
            printf("Correct! +%d points\n", pts);
        } else {
            printf("Wrong! Correct answer was %c\n", questions[idx].correct);
        }
    }

    printf("\nGame Over. Your final score: %d\n", score);
    saveToLeaderboard(name, score);
    remove("saved_game.txt");
}

void playGameFromResume(Question questions[], int totalQuestions, int selected[], int answered, int score, char* name) {
    for (int i = answered; i < 10; i++) {
        int idx = selected[i];
        printf("\nQuestion %d:\n%s", i + 1, questions[idx].question);
        printf("A. " COLOR_GREEN "%s" COLOR_RESET, questions[idx].options[0]);
printf("B. " COLOR_YELLOW "%s" COLOR_RESET, questions[idx].options[1]);
printf("C. " COLOR_BLUE "%s" COLOR_RESET, questions[idx].options[2]);
printf("D. " COLOR_RED "%s" COLOR_RESET, questions[idx].options[3]);

        printf("Enter your answer (A/B/C/D) or X to save & exit: ");
        char answer;
        scanf(" %c", &answer);
        answer = toupper(answer);

        if (answer == 'X') {
            saveGameState(selected, i, score, name);
            printf("Game progress saved! Returning to main menu.\n");
            return;
        }

        if (answer == questions[idx].correct) {
            int pts = questions[idx].difficulty;
            score += pts;
            printf("Correct! +%d points\n", pts);
        } else {
            printf("Wrong! Correct answer was %c\n", questions[idx].correct);
        }
    }

    printf("\nGame Over. Your final score: %d\n", score);
    saveToLeaderboard(name, score);
    remove("saved_game.txt");
}

int resumeGame(Question questions[], int totalQuestions) {
    FILE* f = fopen("saved_game.txt", "r");
    if (!f) return 0;

    int selected[10], answered, score;
    char name[NAME_LENGTH];

    for (int i = 0; i < 10; i++)
        fscanf(f, "%d", &selected[i]);
    fscanf(f, "%d", &answered);
    fscanf(f, "%d", &score);
    fscanf(f, "%s", name);
    fclose(f);

    playGameFromResume(questions, totalQuestions, selected, answered, score, name);
    return 1;
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {
    srand(time(NULL));
    Question questions[MAX_QUESTIONS];
    int totalQuestions = loadQuestions(questions);
    if (totalQuestions == 0) return 1;

    int choice;

    while (1) {
        
        printf("\n=== Trivia Game Menu ===\n");
        printf("1. Start New Game\n");
        printf("2. Resume Game\n");
        printf("3. View Leaderboard\n");
        printf("4. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        clearScreen();

        switch (choice) {
            case 1:
                
                printf("=== New Game ===\n");
                playGame(questions, totalQuestions);
                break;
            case 2:
                
                printf("=== Resume Game ===\n");
                if (!resumeGame(questions, totalQuestions))
                    printf("No saved game found.\n");
                break;
            case 3:
                
                showLeaderboard();
                break;
            case 4:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid option. Try again.\n");
        }
    }

    return 0;
}