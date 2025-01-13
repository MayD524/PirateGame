#pragma once

#ifndef INVENTORY_H
#define INVENTORY_H

#include <items.h>
#include <extended_memory.h>
#include <stdbool.h>

typedef struct {
    GameItem** items; // Array of pointers to items
    int space;        // Maximum number of slots in the inventory
    int count;        // Current number of items in the inventory
} Inventory;

// Function to create an inventory with a specified number of slots
Inventory* Inventory_Create(int space);

// Function to free inventory memory
void Inventory_Destroy(Inventory* inventory);

// Function to check if the inventory has any free space
bool Inventory_HasSpace(Inventory* inventory);

// Function to add an item to a specific slot in the inventory
void Inventory_AddItem(Inventory* inventory, GameItem* item, int slot);

// Function to remove an item from a specific slot in the inventory
void Inventory_RemoveItem(Inventory* inventory, GameItem* item, int slot);

// Function to get an item from a specific slot
GameItem* Inventory_GetItem(Inventory* inventory, int slot);

// Function to check if a specific slot is occupied
bool Inventory_IsSlotOccupied(Inventory* inventory, int slot);

// Function to find the first available slot
int Inventory_FindFirstFreeSlot(Inventory* inventory);

// Function to count how many slots are occupied
int Inventory_CountOccupiedSlots(Inventory* inventory);

// Function to clear all items from the inventory
void Inventory_Clear(Inventory* inventory);

// Function to print inventory details (for debugging)
void Inventory_Print(Inventory* inventory);

#endif
