#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

struct GAMECFG {
    // MAINCFG
    int width = 640;
    int height = 480;

    // var
    int ballRadius = 0;
    int ballCount = 3;
    int ballSpeed = 150;

    // rule
    bool ballCollision = true;
    bool progressiveSpeed = true;

    // radius config
    bool calculateRadius = true;
};

inline int radiusPerBalls(int ballcount) {
    const int baseRes = 200;

    if (ballcount < 1) {
        throw std::runtime_error("WHERES YOUR BALLS");
    }

    return (baseRes * 3) / (2 * ballcount);
}

inline void parseParam(
    GAMECFG& cfg,
    const std::string& category,
    const std::string& parameter,
    const std::string& value
) {
    std::stringstream ss(value);

    if (category == "MAINCFG") {
        int parsedValue;

        if (!(ss >> parsedValue) || parsedValue <= 0) {
            throw std::runtime_error(
                "Invalid MAINCFG value: " + parameter
            );
        }

        std::string extra;
        if (ss >> extra) {
            throw std::runtime_error(
                "Unexpected extra value in MAINCFG: " + parameter
            );
        }

        if (parameter == "WIDTH") {
            cfg.width = parsedValue;
        }
        else if (parameter == "HEIGHT") {
            cfg.height = parsedValue;
        }
        else {
            throw std::runtime_error(
                "Unknown MAINCFG parameter: " + parameter
            );
        }
    }
    else if (category == "var") {
        if (parameter == "BALLCOUNT") {
            int parsedValue;

            if (!(ss >> parsedValue) || parsedValue < 1) {
                throw std::runtime_error(
                    "BALLCOUNT must be at least 1"
                );
            }

            std::string extra;
            if (ss >> extra) {
                throw std::runtime_error(
                    "Invalid BALLCOUNT value"
                );
            }

            cfg.ballCount = parsedValue;
        }
        else if (parameter == "BALLSPEED") {
            int parsedValue;

            if (!(ss >> parsedValue) || parsedValue <= 0) {
                throw std::runtime_error(
                    "BALLSPEED must be greater than 0"
                );
            }

            std::string extra;
            if (ss >> extra) {
                throw std::runtime_error(
                    "Invalid BALLSPEED value"
                );
            }

            cfg.ballSpeed = parsedValue;
        }
        else if (parameter == "BALLRADIUS") {
            if (value == "CALCULATED") {
                cfg.calculateRadius = true;
            }
            else {
                int parsedValue;

                if (!(ss >> parsedValue) || parsedValue <= 0) {
                    throw std::runtime_error(
                        "BALLRADIUS must be a positive integer "
                        "or CALCULATED"
                    );
                }

                std::string extra;
                if (ss >> extra) {
                    throw std::runtime_error(
                        "Invalid BALLRADIUS value"
                    );
                }

                cfg.ballRadius = parsedValue;
                cfg.calculateRadius = false;
            }
        }
        else {
            throw std::runtime_error(
                "Unknown game variable: " + parameter
            );
        }
    }
    else if (category == "rul") {
        int parsedValue;

        if (!(ss >> parsedValue) ||
            (parsedValue != 0 && parsedValue != 1)) {
            throw std::runtime_error(
                "Game rules must be 0 or 1: " + parameter
            );
        }

        std::string extra;
        if (ss >> extra) {
            throw std::runtime_error(
                "Invalid game rule value: " + parameter
            );
        }

        if (parameter == "BALLCOLLISION") {
            cfg.ballCollision = (parsedValue == 1);
        }
        else if (parameter == "PROGRESSIVE_SPEED") {
            cfg.progressiveSpeed = (parsedValue == 1);
        }
        else {
            throw std::runtime_error(
                "Unknown game rule: " + parameter
            );
        }
    }
    else {
        throw std::runtime_error(
            "Unknown configuration category: " + category
        );
    }
}

inline void gameVarInit(GAMECFG& cfg) {
    std::ifstream file("gameConfig.txt");

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open gameConfig.txt"
        );
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::stringstream ss(line);

        std::string category;
        std::string parameter;
        std::string value;

        // Ignore empty lines and comments.
        if (!(ss >> category)) {
            continue;
        }

        if (category[0] == '#') {
            continue;
        }

        if (!(ss >> parameter >> value)) {
            throw std::runtime_error(
                "Invalid configuration syntax at line " +
                std::to_string(lineNumber)
            );
        }

        std::string extra;

        if (ss >> extra) {
            throw std::runtime_error(
                "Unexpected extra token at line " +
                std::to_string(lineNumber)
            );
        }

        try {
            parseParam(cfg, category, parameter, value);
        }
        catch (const std::exception& e) {
            throw std::runtime_error(
                "Config error at line " +
                std::to_string(lineNumber) +
                ": " + e.what()
            );
        }
    }

    if (cfg.calculateRadius) {
        cfg.ballRadius = radiusPerBalls(cfg.ballCount);
    }
}
