#pragma once
#include <string>
#include <optional>
#include "ItemType.h"

char getch();
bool kbhit();
void drain_input();
int read_int_safe(int fallback = 0);
void wait_key();
std::optional<ItemType> map_key_to_item(char ch);