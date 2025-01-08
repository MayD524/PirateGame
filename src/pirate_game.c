#include <pirate_game.h>

PirateGame* PirateGame_CreateGame() {
    PirateGame* game = safe_malloc(sizeof(PirateGame));

    return game;
}