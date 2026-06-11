    int day = tm->tm_mday;

    habits[idx].done_today = 1;
    habits[idx].calendar[day - 1] = 'G';

    if (habits[idx].streak > habits[idx].best_streak)
        habits[idx].best_streak = habits[idx].streak;
}

// ===== MAIN =====
int main() {
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    init_sample();

    int height = 20, width = 30;

    WINDOW *menu = newwin(height, width, 0, 0);
    WINDOW *main = newwin(height, 70, 0, 30);

    int ch;

    while ((ch = getch()) != 'q') {

        draw_menu(menu);

        if (mode == 0)
            draw_list(main);
        else
            draw_detail(main, selected);

        switch (ch) {

            case KEY_UP:
                if (selected > 0) selected--;
                break;

            case KEY_DOWN:
                if (selected < habit_count - 1) selected++;
                break;

            case '\n':
                mode = 1;
                break;

            case 'm':
                mark_done();
                break;

            case 'b':
                mode = 0;
                break;
        }
    }

    endwin();
    return 0;
}