#pragma once

#include <fstream>
#include <string>
#include <iostream>

struct hsvBarrier{
    std::string colname;
    int huemin = 0,
        huemax = 0;
    
    int satmin = 0,
        satmax = 0;

    int valmin = 0,
        valmax = 0;
    
};

/*
file itself is in the form of
    COLNAME
    HUEMIN HUEMAX
    SATMIN SATMAX
    VALMIN VALMAX
*/

// >>> WILL THROW EXCEPTION
int parseColour(std::ifstream &colourConfig, hsvBarrier &barrier) {
    if (!colourConfig.is_open()) {
        return 1;
    }

    auto &[cname, hm, hx, sm, sx, vm, vx] = barrier;

    // if (!(colourConfig >> cname >> hm >> hx >> sm >> sx >> vm >> vx)) {
    //     return 2;
    // }
    colourConfig >> cname >> hm >> hx >> sm >> sx >>vm >> vx;

    return 0;
}
