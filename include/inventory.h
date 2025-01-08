#pragma once

#ifndef INVENTORY_H
#define INVENTORY_H

#include <items.h>
#include <extended_memory.h>

typedef struct {
    GameItem** items;
    int space;
} Inventory;

Inventory* Inventory_Create(int space);

void Inventory_AddItem(GameItem* item, int slot);
void Inventory_RemoveItem(GameItem* item, int slot);

#endif