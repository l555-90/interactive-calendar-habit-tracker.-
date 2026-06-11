#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>  


#define MAX_HABITS 10         
#define NAME_SIZE  30      
// Define this once. Use double backslashes for Windows pathing.
#define BUZZER_PATH "C:\\Users\\User I55-90\\OneDrive\\Desktop\\vscode\\habit tracker calander\\buzzer.wav"  

struct Habit {
    char name[NAME_SIZE];   
    int streak;             
    int best_streak;        
    int done_today;         
    int total_done;  
    int completed_days[31]; 
};

struct Habit habits[MAX_HABITS];  
int habit_count = 0;
int selected_habit_index = -1; 
int active_option = -1; 
int current_virtual_day = 0; 

WINDOW *menu_win;
WINDOW *calendar_win;

void draw_layout();
void update_calendar_view();
void mark_done_workflow();

void save_habits() {
    FILE *file = fopen("habits.dat", "wb");
    if (file == NULL) return;
    fwrite(&habit_count, sizeof(int), 1, file);
    fwrite(&current_virtual_day, sizeof(int), 1, file); 
    fwrite(habits, sizeof(struct Habit), habit_count, file); 
    fclose(file);
}

void load_habits() {
    FILE *file = fopen("habits.dat", "rb");
    if (file == NULL) return;
    fread(&habit_count, sizeof(int), 1, file);
    fread(&current_virtual_day, sizeof(int), 1, file); 
    fwrite(habits, sizeof(struct Habit), habit_count, file); // Fixed write mismatch to load safely
    fclose(file);
}

void draw_layout() {
    clear();
    refresh();

    box(menu_win, 0, 0);
    mvwprintw(menu_win, 0, 2, " ACTIONS & INVENTORY ");
    wrefresh(menu_win);

    box(calendar_win, 0, 0);
    mvwprintw(calendar_win, 0, 2, " CALENDAR & METRICS ");
    wrefresh(calendar_win);
}

void update_calendar_view() {
    for (int i = 1; i < 21; i++) {
        mvwprintw(calendar_win, i, 1, "                                        ");
    }

    time_t now = time(NULL);
    struct tm *today = localtime(&now);
    int current_day   = current_virtual_day; 
    int current_month = today->tm_mon;
    int current_year  = today->tm_year + 1900;

    char *months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (current_month == 1 && ((current_year % 4 == 0 && current_year % 100 != 0) || (current_year % 400 == 0))) {
        days_in_month[1] = 29;
    }

    struct tm first_day_tm = *today;
    first_day_tm.tm_mday = 1;
    mktime(&first_day_tm);
    int start_weekday = first_day_tm.tm_wday;

    if (selected_habit_index == -1 || habit_count == 0) {
        mvwprintw(calendar_win, 2, 3, "SYSTEM: Overview Mode");
        mvwprintw(calendar_win, 4, 3, "--- %s %d ---", months[current_month], current_year);
        mvwprintw(calendar_win, 6, 3, "Su  Mo  Tu  We  Th  Fr  Sa");

        int row = 7;
        int col_offset = 3;
        int current_col = start_weekday;

        for (int day = 1; day <= days_in_month[current_month]; day++) {
            int x_pos = col_offset + (current_col * 4);

            if (day == current_day) {
                wattron(calendar_win, A_REVERSE | A_BOLD);
                mvwprintw(calendar_win, row, x_pos, "[%2d]", day);
                wattroff(calendar_win, A_REVERSE | A_BOLD);
            } else {
                mvwprintw(calendar_win, row, x_pos, " %2d ", day);
            }

            current_col++;
            if (current_col > 6) {
                current_col = 0;
                row++;
            }
        }

        box(calendar_win, 0, 0);
        wrefresh(calendar_win);
        return;
    }

    struct Habit h = habits[selected_habit_index];

    mvwprintw(calendar_win, 2, 3, "HABIT: %s", h.name);
    mvwprintw(calendar_win, 4, 3, "Current Streak: %d days", h.streak);
    mvwprintw(calendar_win, 5, 3, "Best Streak   : %d days", h.best_streak);
    mvwprintw(calendar_win, 6, 3, "Total Done    : %d times", h.total_done);
    
    mvwprintw(calendar_win, 7, 3, "Status Today  : ");
    if (h.done_today) {
        wattron(calendar_win, COLOR_PAIR(1) | A_BOLD);
        wprintw(calendar_win, "DONE");
        wattroff(calendar_win, COLOR_PAIR(1) | A_BOLD);
    } else {
        wattron(calendar_win, COLOR_PAIR(2) | A_BOLD);
        wprintw(calendar_win, "NOT DONE");
        wattroff(calendar_win, COLOR_PAIR(2) | A_BOLD);
    }

    mvwprintw(calendar_win, 10, 3, "--- %s %d ---", months[current_month], current_year);
    mvwprintw(calendar_win, 12, 3, "Su  Mo  Tu  We  Th  Fr  Sa");

    int row = 13;
    int col_offset = 3;
    int current_col = start_weekday;

    for (int day = 1; day <= days_in_month[current_month]; day++) {
        int x_pos = col_offset + (current_col * 4);
        int state = h.completed_days[day - 1];

        if (state == 1) {
            wattron(calendar_win, COLOR_PAIR(1) | A_BOLD);
            mvwprintw(calendar_win, row, x_pos, "[%2d]", day); 
            wattroff(calendar_win, COLOR_PAIR(1) | A_BOLD);
        } else if (state == 2) {
            wattron(calendar_win, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(calendar_win, row, x_pos, "[%2d]", day); 
            wattroff(calendar_win, COLOR_PAIR(2) | A_BOLD);
        } else {
            mvwprintw(calendar_win, row, x_pos, " %2d ", day); 
        }

        current_col++;
        if (current_col > 6) {
            current_col = 0;
            row++;
        }
    }

    mvwprintw(calendar_win, 19, 3, "[##] ");
    wattron(calendar_win, COLOR_PAIR(1));
    wprintw(calendar_win, "Done");
    wattroff(calendar_win, COLOR_PAIR(1));

    wprintw(calendar_win, "    [##] ");
    wattron(calendar_win, COLOR_PAIR(2) | A_BOLD);
    wprintw(calendar_win, "Missed");
    wattroff(calendar_win, COLOR_PAIR(2) | A_BOLD);

    box(calendar_win, 0, 0); 
    wrefresh(calendar_win);
}

void clean_menu_input_area() {
    for (int i = 12; i < 21; i++) {
        mvwprintw(menu_win, i, 1, "                                        ");
    }
    box(menu_win, 0, 0);
    wrefresh(menu_win);
}

void show_yellow_header(const char *label) {
    wattron(menu_win, COLOR_PAIR(3) | A_BOLD);
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 0, 2, " ACTIONS & INVENTORY [%s] ", label);
    wattroff(menu_win, COLOR_PAIR(3) | A_BOLD);
}

void reset_normal_header() {
    wattron(menu_win, COLOR_PAIR(0));
    box(menu_win, 0, 0);
    mvwprintw(menu_win, 0, 2, " ACTIONS & INVENTORY ");
    wattroff(menu_win, COLOR_PAIR(0));
}

void draw_navigation_panel() {
    mvwprintw(menu_win, 2,  3, "====== NAV PANEL ======");
    
    if (active_option == 1) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 4,  3, "1. Choose/Show Habit    ");
    if (active_option == 1) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 2) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 5,  3, "2. Add New Habit        ");
    if (active_option == 2) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 3) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 6,  3, "3. Mark Selected as Done");
    if (active_option == 3) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 4) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 7,  3, "4. Undo Mark Done       ");
    if (active_option == 4) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 5) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 8,  3, "5. Start New Day        ");
    if (active_option == 5) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 6) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 9,  3, "6. Delete Selected Habit");
    if (active_option == 6) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    if (active_option == 0) wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(menu_win, 10, 3, "0. Exit Application     ");
    if (active_option == 0) wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);

    mvwprintw(menu_win, 11, 3, "=======================");
    mvwprintw(menu_win, 20, 3, "Command Selection: ");
    wrefresh(menu_win);
}

void select_habit_workflow() {
    clean_menu_input_area();
    if (habit_count == 0) {
        mvwprintw(menu_win, 14, 2, "No habits created yet!");
        wrefresh(menu_win);
        return;
    }

    show_yellow_header("SELECTING");
    mvwprintw(menu_win, 13, 2, "--- CHOOSE A HABIT ---");
    for (int i = 0; i < habit_count; i++) {
        mvwprintw(menu_win, 14 + i, 2, "%d. %s", i + 1, habits[i].name);
    }
    mvwprintw(menu_win, 15 + habit_count, 2, "Enter choice (1-%d): ", habit_count);
    wrefresh(menu_win);

    echo();
    char input[10];
    mvwgetnstr(menu_win, 15 + habit_count, 22, input, sizeof(input) - 1);
    noecho();

    int choice = atoi(input);
    if (choice >= 1 && choice <= habit_count) {
        selected_habit_index = choice - 1; 
    }
    reset_normal_header();
    clean_menu_input_area();
}

void add_habit_workflow() {
    clean_menu_input_area();
    if (habit_count >= MAX_HABITS) {
        mvwprintw(menu_win, 13, 2, "Max capacity reached!");
        wrefresh(menu_win);
        return;
    }

    show_yellow_header("CREATING");
    selected_habit_index = -1; 
    update_calendar_view();

    mvwprintw(menu_win, 13, 2, "Enter habit name: ");
    wrefresh(menu_win);

    echo();
    char name_buf[NAME_SIZE];
    mvwgetnstr(menu_win, 13, 20, name_buf, NAME_SIZE - 1);
    noecho();

    name_buf[strcspn(name_buf, "\n")] = 0;

    if (strlen(name_buf) > 0) {
        strcpy(habits[habit_count].name, name_buf);
        habits[habit_count].streak = 0;
        habits[habit_count].best_streak = 0;
        habits[habit_count].done_today = 0;
        habits[habit_count].total_done = 0;
        memset(habits[habit_count].completed_days, 0, sizeof(habits[habit_count].completed_days));
        habit_count++;
        save_habits();
    }

    clean_menu_input_area();
    show_yellow_header("UPDATED LIST");
    mvwprintw(menu_win, 13, 2, "--- CURRENT HABITS ---");
    for (int i = 0; i < habit_count; i++) {
        mvwprintw(menu_win, 14 + i, 2, "%d. %s", i + 1, habits[i].name);
    }
    wrefresh(menu_win);
    wtimeout(menu_win, 1500);
    wgetch(menu_win);
    wtimeout(menu_win, -1);

    reset_normal_header();
    clean_menu_input_area();
}

void mark_done_workflow() {
    clean_menu_input_area();
    if (habit_count == 0) {
        mvwprintw(menu_win, 14, 2, "No habits created yet!");
        wrefresh(menu_win);
        wtimeout(menu_win, 1200);
        wgetch(menu_win);
        wtimeout(menu_win, -1);
        return;
    }

    show_yellow_header("MARK DONE");
    selected_habit_index = -1; 
    update_calendar_view();

    mvwprintw(menu_win, 13, 2, "--- CHOOSE TO MARK DONE ---");
    for (int i = 0; i < habit_count; i++) {
        mvwprintw(menu_win, 14 + i, 2, "%d. %s [%s]", i + 1, habits[i].name, habits[i].done_today ? "DONE" : "PENDING");
    }
    mvwprintw(menu_win, 15 + habit_count, 2, "Enter choice (1-%d): ", habit_count);
    wrefresh(menu_win);

    echo();
    char input[10];
    mvwgetnstr(menu_win, 15 + habit_count, 22, input, sizeof(input) - 1);
    noecho();

    int choice = atoi(input);
    if (choice < 1 || choice > habit_count) {
        reset_normal_header();
        clean_menu_input_area();
        return; 
    }

    int idx = choice - 1;
    selected_habit_index = idx; 
    
    if (habits[idx].done_today == 1) {
        reset_normal_header();
        clean_menu_input_area();
        mvwprintw(menu_win, 13, 2, "Already marked done today!");
        wrefresh(menu_win);
        wtimeout(menu_win, 1200);
        wgetch(menu_win);
        wtimeout(menu_win, -1);
        return;
    }

    habits[idx].done_today = 1;
    int day = current_virtual_day;
    
    if (day >= 1 && day <= 31) {
        habits[idx].completed_days[day - 1] = 1; 
    }

    habits[idx].streak++;
    habits[idx].total_done++;
    if (habits[idx].streak > habits[idx].best_streak) {
        habits[idx].best_streak = habits[idx].streak;
    }

    save_habits();
    update_calendar_view();

    reset_normal_header();
    clean_menu_input_area();
}

void unmark_done_workflow() {
    clean_menu_input_area();
    if (habit_count == 0) {
        mvwprintw(menu_win, 14, 2, "No habits created yet!");
        wrefresh(menu_win);
        return;
    }

    show_yellow_header("UNDO DONE");
    selected_habit_index = -1; 
    update_calendar_view();

    mvwprintw(menu_win, 13, 2, "--- CHOOSE TO UNDO DONE ---");
    for (int i = 0; i < habit_count; i++) {
        mvwprintw(menu_win, 14 + i, 2, "%d. %s [%s]", i + 1, habits[i].name, habits[i].done_today ? "DONE" : "PENDING");
    }
    mvwprintw(menu_win, 15 + habit_count, 2, "Enter choice (1-%d): ", habit_count);
    wrefresh(menu_win);

    echo();
    char input[10];
    mvwgetnstr(menu_win, 15 + habit_count, 22, input, sizeof(input) - 1);
    noecho();

    int choice = atoi(input);
    if (choice >= 1 && choice <= habit_count) {
        int idx = choice - 1;
        selected_habit_index = idx; 

        if (habits[idx].done_today == 0) {
            reset_normal_header();
            clean_menu_input_area();
            mvwprintw(menu_win, 13, 2, "Habit wasn't done today.");
            wrefresh(menu_win);
            wtimeout(menu_win, 1200);
            wgetch(menu_win);
            wtimeout(menu_win, -1);
            return;
        }

        habits[idx].done_today = 0;
        habits[idx].streak--;
        habits[idx].total_done--;

        int day = current_virtual_day;
        habits[idx].completed_days[day - 1] = 0; 

        save_habits();
        update_calendar_view();
    }
    reset_normal_header();
    clean_menu_input_area();
}

void reset_today_workflow() {
    clean_menu_input_area();
    show_yellow_header("NEW DAY");
    
    selected_habit_index = -1; 
    update_calendar_view();

    int missed_count = 0;
    int pending_indices[MAX_HABITS];
    for (int i = 0; i < habit_count; i++) {
        if (habits[i].done_today == 0) {
            pending_indices[missed_count] = i;
            missed_count++;
        }
    }

    if (missed_count > 0) {
        wattron(menu_win, COLOR_PAIR(2) | A_BOLD);
        mvwprintw(menu_win, 13, 2, "!!! WARNING: UNMARKED HABITS !!!");
        wattroff(menu_win, COLOR_PAIR(2) | A_BOLD);
        // ... (rest of your logic to print missed habits)
    } 
    
    // ... (rest of your input logic)

    if (atoi(input) == 1) {
        // ... (your logic for resetting streaks and updating days)
        
        current_virtual_day++; 
        save_habits();
        
        clean_menu_input_area();
        show_yellow_header("NEW DAY");
        mvwprintw(menu_win, 14, 2, "Calendar advanced successfully!");
        
        if (trigger_alarm) {
    wattron(menu_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(menu_win, 16, 2, "DEBUG: TRIGGERING ALARM...");
    wrefresh(menu_win);
    
    // Use the absolute, foolproof command
// Replace 'YOUR_WINDOWS_USERNAME' and ensure this path is exact
// The C compiler will replace BUZZER_PATH with your string automatically
system("powershell.exe -c (New-Object Media.SoundPlayer 'buzzer.wav').PlaySync()");  wtimeout(menu_win, -1);
} else {
        clean_menu_input_area();
        show_yellow_header("NEW DAY");
        mvwprintw(menu_win, 13, 2, "Action cancelled.");
        wtimeout(menu_win, 1000);
        wgetch(menu_win);
        wtimeout(menu_win, -1);
    }
    
    reset_normal_header();
    clean_menu_input_area();
}

void delete_habit_workflow() {
    clean_menu_input_area();
    if (habit_count == 0) {
        mvwprintw(menu_win, 14, 2, "No habits created yet!");
        wrefresh(menu_win);
        return;
    }

    show_yellow_header("DELETING");
    selected_habit_index = -1; 
    update_calendar_view();

    mvwprintw(menu_win, 13, 2, "--- SELECT HABIT TO DELETE ---");
    for (int i = 0; i < habit_count; i++) {
        mvwprintw(menu_win, 14 + i, 2, "%d. %s", i + 1, habits[i].name);
    }
    mvwprintw(menu_win, 15 + habit_count, 2, "Enter choice (1-%d): ", habit_count);
    wrefresh(menu_win);

    echo();
    char input[10];
    mvwgetnstr(menu_win, 15 + habit_count, 22, input, sizeof(input) - 1);
    noecho();

    int choice = atoi(input);
    if (choice >= 1 && choice <= habit_count) {
        int target_idx = choice - 1;
        
        for (int i = target_idx; i < habit_count - 1; i++) {
            habits[i] = habits[i + 1];
        }
        habit_count--;
        save_habits();
    }

    clean_menu_input_area();
    show_yellow_header("UPDATED LIST");
    if (habit_count == 0) {
        mvwprintw(menu_win, 13, 2, "No remaining active habits.");
    } else {
        mvwprintw(menu_win, 13, 2, "--- REMAINING INVENTORY ---");
        for (int i = 0; i < habit_count; i++) {
            mvwprintw(menu_win, 14 + i, 2, "%d. %s", i + 1, habits[i].name);
        }
    }
    wrefresh(menu_win);
    wtimeout(menu_win, 1500);
    wgetch(menu_win);
    wtimeout(menu_win, -1);

    reset_normal_header();
    clean_menu_input_area();
}

int main() {
    load_habits();

    if (current_virtual_day == 0) {
        time_t now = time(NULL);
        struct tm *today = localtime(&now);
        current_virtual_day = today->tm_mday;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  
        init_pair(2, COLOR_RED, COLOR_BLACK);    
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); 
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK); 
    }

    menu_win     = newwin(22, 42, 0, 0);
    calendar_win = newwin(22, 42, 0, 43);

    int choice = -1;
    while (choice != 0) {
        draw_layout();
        update_calendar_view(); 
        
        active_option = -1; 
        draw_navigation_panel();

        echo();
        char menu_input[10];
        mvwgetnstr(menu_win, 20, 22, menu_input, sizeof(menu_input) - 1);
        noecho();
        choice = atoi(menu_input);

        active_option = choice; 
        draw_navigation_panel(); 

        if (choice == 1)      select_habit_workflow();
        else if (choice == 2) add_habit_workflow();
        else if (choice == 3) mark_done_workflow();
        else if (choice == 4) unmark_done_workflow();
        else if (choice == 5) reset_today_workflow();
        else if (choice == 6) delete_habit_workflow();
        else if (choice == 0) break;
    }

    endwin();
    printf("Data written safely. Goodbye!\n");
    return 0;
}