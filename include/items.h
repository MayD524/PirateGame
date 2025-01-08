#pragma once

#ifndef ITEMS_H
#define ITEMS_H

#define ROOT_ITEM_TABLE_SIZE 1

#include <models.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY
} ItemRarity;

#define SLOT_HEAD    (1 << 0)
#define SLOT_BODY    (1 << 1)
#define SLOT_HANDS   (1 << 2)
#define SLOT_FEET    (1 << 3)
#define SLOT_WEAPON  (1 << 4)
#define SLOT_OFFHAND (1 << 5)
#define SLOT_HIP     (1 << 6)

#define CATEGORY_WEAPON       (1 << 0)
#define CATEGORY_ARMOR        (1 << 1)
#define CATEGORY_CONSUMABLE   (1 << 2)
#define CATEGORY_TOOL         (1 << 3)
#define CATEGORY_KEY_ITEM     (1 << 4)
#define CATEGORY_GENERIC      (1 << 5)

typedef struct {
    const char* item_name;
    const char* model_path;
    const int model_id;
    const char* item_image_path;

    int category;
    const char* description;
    int durability;
    int value;
    ItemRarity rarity;

    float weight;
    float size;

    bool is_usable;
    bool is_equippable;
    int slot;
    void (*useEffect)(void);

    const char* localized_name;
    void* custom_data;
} GameItem;

extern const GameItem root_item_table[ROOT_ITEM_TABLE_SIZE];

#endif