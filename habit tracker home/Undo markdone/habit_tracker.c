
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_HABITS 10        
#define NAME_SIZE  50       
// inside option 1: 
struct Habit {
    char name[NAME_SIZE];   
    int streak;             //continuous days
    int best_streak;        
    int done_today;         //if done, mark as done is not executed
    int total_done;         //count regardless of streaks
};

// 
struct Habit habits[MAX_HABITS];  //each habits has atttribute of habit struct
int habit_count = 0;              


// write habits into file, for option 1
void save_habits() {
    FILE *file = fopen("habits.dat", "wb"); //declare pointer to file

    if (file == NULL) { // if fail to open
        printf("Error: cannot save file!\n");
        return;
    }

    fwrite(&habit_count, sizeof(int), 1, file); // writes the total habit count,  if 2
    fwrite(habits, sizeof(struct Habit), habit_count, file); // writes count,streaks, name of habit 1, habit 2 if habit ocunt is 2. 

    fclose(file);
    printf("Saved!\n");
}

// loads into the memory, after ram it's over so even if vscode over, it works for option 1
void load_habits() {
    FILE *file = fopen("habits.dat", "rb");

    if (file == NULL) { //faiil to open if first time run, nothing is in habits.dat
        printf("No save file found. Starting fresh!\n");
        return;
    }

    fread(&habit_count, sizeof(int), 1, file);
    fread(habits, sizeof(struct Habit), habit_count, file);

    fclose(file);
    printf("Loaded %d habits!\n", habit_count);
}
// option 2 -------------------------------------------------


void add_habit() {
    if (habit_count >= MAX_HABITS) {
        printf("Cannot add more than %d habits!\n", MAX_HABITS);
        return;
    }

    printf("Enter habit name: ");
    scanf(" %[^\n]", habits[habit_count].name); //%s would stop at space but this doesnt. habits[0].name is "drinking"

    habits[habit_count].streak      = 0; //habit[0]
    habits[habit_count].best_streak = 0;
    habits[habit_count].done_today  = 0;
    habits[habit_count].total_done  = 0;

    habit_count++;

    printf("Habit '%s' added!\n", habits[habit_count - 1].name);
    save_habits();
}

// otion 1
void show_habits() {
    if (habit_count == 0) {
        printf("You have no habits yet! Add your first one.\n");
        return;
    }

    printf("\n");
    printf("================================\n");
    printf("         YOUR HABITS\n");
    printf("================================\n");

    for (int i = 0; i < habit_count; i++) {
        char *fire  = (habits[i].streak > 0) ? " ***FIRE***" : ""; //fire for greater than 0 day
        char *check = habits[i].done_today ? "[DONE]" : "[not done]";// ternary condition

        printf("%d. %s\n", i + 1, habits[i].name); //starts with 1, not 0. 1. drinking
        printf("   Streak     : %d days%s\n", habits[i].streak, fire); // %d is called from struct, value is modified by mark_done() and reset_today()
        printf("   Best streak: %d days\n", habits[i].best_streak);
        printf("   Total done : %d times\n", habits[i].total_done);
        printf("   Today      : %s\n", check);
        printf("--------------------------------\n");
    }
}

// Mark habit as done/ OPTION 3
void mark_done() {
    if (habit_count == 0) {
        printf("No habits!\n");
        return;
    }

    show_habits();

    printf("Enter habit number (1-%d): ", habit_count); //choice
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > habit_count) {
        printf("Wrong number!\n");
        return;
    }

    int index = choice - 1; //if they choose habit no 2. then it's habit [1] in index. 

    if (habits[index].done_today == 1) {
        printf("You already completed this habit today!\n");
        return;
    }

    habits[index].done_today = 1;
    habits[index].streak++;
    habits[index].total_done++;

    if (habits[index].streak > habits[index].best_streak) { //update best streak
        habits[index].best_streak = habits[index].streak;
    }

    printf("Great! '%s' done!\n", habits[index].name);
    printf("Streak: %d days in a row!\n", habits[index].streak);

    if (habits[index].streak == 7) {
        printf("ONE WEEK STRAIGHT! You are awesome!\n");
    } else if (habits[index].streak == 30) {
        printf("30 DAYS! It is a real habit now!\n");
    } else if (habits[index].streak == 100) {
        printf("100 DAYS! You are a legend!\n");
    }

    save_habits();
}
// Undo mark done - option 6
void unmark_done() {
    if (habit_count == 0) {
        printf("No habits!\n");
        return;
    }

    show_habits();

    printf("Enter habit number to undo (1-%d): ", habit_count);
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > habit_count) {
        printf("Wrong number!\n");
        return;
    }

    int index = choice - 1;

    if (habits[index].done_today == 0) {
        printf("This habit was not marked done today!\n");
        return;
    }

    habits[index].done_today = 0;
    habits[index].streak--;
    habits[index].total_done--;

    // fix best streak if needed
    if (habits[index].streak < habits[index].best_streak) {
        // best streak stays, only current streak goes down
    }

    printf("Undone! '%s' marked as not done.\n", habits[index].name);
    save_habits();
}

///------------------
// Resets "done today" - and new day starts option 4
void reset_today() {
    printf("Start a new day? (1 = yes, 0 = no): ");
    int confirm;
    scanf("%d", &confirm);

    if (confirm != 1) {
        printf("Cancelled.\n");
        return;
    }
// skip the day without mark done. streak is reset to 0. 
    for (int i = 0; i < habit_count; i++) {
        if (habits[i].done_today == 0 && habits[i].streak > 0) {// if today is not markdone and streak>0, streak reset
            printf("Streak for '%s' reset! Was %d days.\n",
                   habits[i].name, habits[i].streak);
            habits[i].streak = 0; //streak resert
        }
        habits[i].done_today = 0;// new day means; done today=0 for every element
    }

    save_habits();
    printf("New day started! Good luck!\n");
}

// Delete a habit option
void delete_habit() {
    if (habit_count == 0) {
        printf("No habits to delete!\n");
        return;
    }

    show_habits();

    printf("Enter habit number to delete (1-%d): ", habit_count);
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > habit_count) {
        printf("Wrong number!\n");
        return;
    }

    int index = choice - 1;
    printf("Delete '%s'? (1 = yes, 0 = no): ", habits[index].name);
    int confirm;
    scanf("%d", &confirm);

    if (confirm != 1) {
        printf("Cancelled.\n");
        return;
    }

    for (int i = index; i < habit_count - 1; i++) {
        habits[i] = habits[i + 1]; // if to remove habit 1, then after deletion habit 1 will have value of habit 2. habit 2= habit 3. and last habit to be delted. 
    }                          

    habit_count--;            
    printf("Habit deleted!\n");
    save_habits();
}                              


// Show ASCII calendar for current month
/*void show_calendar() {
    time_t now = time(NULL);
    struct tm *today = localtime(&now);

    int current_day   = today->tm_mday;
    int current_month = today->tm_mon;
    int current_year  = today->tm_year + 1900;

    char *months[] = {
        "January", "February", "March",    "April",
        "May",     "June",     "July",     "August",
        "September","October", "November", "December"
    };

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // leap year check
    if ((current_year % 4 == 0 && current_year % 100 != 0) || current_year % 400 == 0) {
        days_in_month[1] = 29;
    }

    // find start day of month
    today->tm_mday = 1;
    mktime(today);
    int start_day = today->tm_wday;
    start_day = (start_day == 0) ? 6 : start_day - 1;  // Mon=0

    printf("\n");
    printf("================================\n");
    printf("     %s %d\n", months[current_month], current_year);
    printf("================================\n");
    printf(" Mo  Tu  We  Th  Fr  Sa  Su\n");
    printf("--------------------------------\n");

    for (int i = 0; i < start_day; i++) {
        printf("    ");
    }

    int col = start_day;

    for (int day = 1; day <= days_in_month[current_month]; day++) {
        if (day == current_day) {
            printf("[%2d]", day);  // today in brackets
        } else {
            printf(" %2d ", day);
        }

        col++;
        if (col == 7) {
            printf("\n");
            col = 0;
        }
    }

    printf("\n================================\n");
    printf("Today: %s %d, %d\n", months[current_month], current_day, current_year);
}*/


// Main menu
void show_menu() {
    printf("\n");
    printf("================================\n");
    printf("         HABIT TRACKER\n");
    printf("================================\n");
    printf("1. Show habits\n");
    printf("2. Add habit\n");
    printf("3. Mark as done\n");
    printf("4. Undo mark done\n");
    printf("5. Start new day\n");
    printf("6. Delete habit\n");
    
    // printf("6. Show calendar\n");
    printf("0. Exit\n");
    printf("================================\n");
    printf("Choose: ");
}

// =====================

// =====================
int main() {
    printf("Hello! This is your Habit Tracker!\n");

    load_habits();

    int choice = -1;

    while (choice != 0) {
        show_menu();
        scanf("%d", &choice);

        if (choice == 1) {
            show_habits();
        } else if (choice == 2) {
            add_habit();
        } else if (choice == 3) {
            mark_done();
        } else if (choice == 4) {
            unmark_done();
        }else if (choice == 5) {
            reset_today();
        } else if (choice == 6) {
            delete_habit();
        // } else if (choice == 6) {
        //     show_calendar();
        } else if (choice == 0) {
            printf("Bye! Don't forget your habits!\n");
        } else {
            printf("Wrong choice! Try again.\n");
        }
    }

    return 0;
}
