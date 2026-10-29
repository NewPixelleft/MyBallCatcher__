#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>

struct hsvBarrier {
    std::string colname;
    int hsvmin[3] = {-1, -1, -1};
    int hsvmax[3] = {-1, -1, -1};
};

/*
file itself is in the form of
    COLNAME
    HSV_MIN H S V
    HSV_MAX H S V
*/

// >>> WILL THROW EXCEPTION
void parseColour(std::ifstream& colourConfig, hsvBarrier& barrier) {
    std::string buf;
    std::string pickedColour;

    bool isPicked = false;
    bool doesPickedExist = false;

    while (std::getline(colourConfig, buf)) {
        std::stringstream ss(buf);

        std::string indicator;
        ss >> indicator;

        if (indicator.empty() || indicator[0] == '#') {
            continue;
        }

        if (indicator == "pick") {
            if (isPicked) {
                throw std::runtime_error(
                    "You cannot write 'pick' twice in colourConfig.txt"
                );
            }

            if (!(ss >> pickedColour)) {
                throw std::runtime_error(
                    "Missing colour name after 'pick'"
                );
            }

            isPicked = true;
            continue;
        }

        if (indicator == pickedColour && isPicked) {
            doesPickedExist = true;
            barrier.colname = pickedColour;

            bool existHSVmin = false;
            bool existHSVmax = false;

            auto lambdaBarrierParse = [&](const std::string& line) -> void {
                std::stringstream ssCOLPARSER(line);

                std::string barrierIndicator;

                if (!(ssCOLPARSER >> barrierIndicator)) {
                    return; // Ignore empty lines
                }

                if (barrierIndicator[0] == '#') {
                    return; // Ignore comments
                }

                if (barrierIndicator == "HSV_MIN" && !existHSVmin) {
                    if (!(ssCOLPARSER >> barrier.hsvmin[0]
                                      >> barrier.hsvmin[1]
                                      >> barrier.hsvmin[2])) {
                        throw std::runtime_error(
                            "INVALID PARSING OF COLOUR on HSV_MIN"
                        );
                    }

                    existHSVmin = true;
                }
                else if (barrierIndicator == "HSV_MAX" && !existHSVmax) {
                    if (!(ssCOLPARSER >> barrier.hsvmax[0]
                                      >> barrier.hsvmax[1]
                                      >> barrier.hsvmax[2])) {
                        throw std::runtime_error(
                            "INVALID PARSING OF COLOUR in HSV_MAX"
                        );
                    }

                    existHSVmax = true;
                }
                else {
                    throw std::runtime_error("Colour Parsing Failed");
                }
            };

            while (std::getline(colourConfig, buf)) {
                std::stringstream sectionStream(buf);

                std::string sectionIndicator;
                sectionStream >> sectionIndicator;

                if (sectionIndicator.empty() || sectionIndicator[0] == '#') {
                    continue;
                }

                // Stop when the next colour section begins.
                if (sectionIndicator != "HSV_MIN" &&
                    sectionIndicator != "HSV_MAX") {
                    break;
                }

                lambdaBarrierParse(buf);

                if (existHSVmin && existHSVmax) {
                    return;
                }
            }

            if (!existHSVmin || !existHSVmax) {
                throw std::runtime_error(
                    "Selected colour is missing HSV_MIN or HSV_MAX"
                );
            }

            return;
        }
    }

    if (!isPicked) {
        throw std::runtime_error(
            "No 'pick' directive found in colourConfig.txt"
        );
    }

    if (!doesPickedExist) {
        throw std::runtime_error(
            "Selected colour does not exist in colourConfig.txt"
        );
    }
}
