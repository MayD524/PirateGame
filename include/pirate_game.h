#pragma once

#ifndef PIRATE_GAME_H
#define PIRATE_GAME_H

#include <water_plane.h>
#include <extended_memory.h>

typedef struct {
    WaterPlane* waterPlane;
} PirateGame;

PirateGame* PirateGame_CreateGame();

#endif