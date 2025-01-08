#include <inventory.h>

Inventory* Inventory_Create(int space) {
    Inventory* inv = safe_malloc(sizeof(Inventory));

    inv->items = safe_malloc(space * sizeof(GameItem*));
    inv->space = space;
}