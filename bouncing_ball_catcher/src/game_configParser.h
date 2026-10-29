#pragma once
#include <fstream>

struct GAMECFG{
    int ballRadius = 0;
    int ballCount = 0;
};

int radiusPerBalls(int ballcount){
    const int basreRes = 140;
    if (ballcount < 1){
        throw std::runtime_error("WHERES YOUR BALLS");
    }
    int s1 = basreRes / ballcount;
    return s1 * 3 / 2;
}

void gameVarInit(GAMECFG &cfg) {
    std::ifstream file("gameConfig.txt");

    if (!file.is_open()) {
        throw std::runtime_error("Could not open gameConfig.txt");
    }

    if (!(file >> cfg.ballCount)) {
        throw std::runtime_error("WHERES YOUR BALLL");
    }
    cfg.ballRadius= radiusPerBalls(cfg.ballCount);
}