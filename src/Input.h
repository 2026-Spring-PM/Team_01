#pragma once

char getch();
bool kbhit();
void drain_input();
int read_int_safe(int fallback = 0);
void wait_key();
