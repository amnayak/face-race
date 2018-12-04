#pragma once
#include <string>
#include <vector>

class Dialogue {
    public:
        Dialogue();
        ~Dialogue();

    struct Choice {
        std::string text;
        uint32_t goto_index; //index into Lines vector
        int points; //each dialogue has associated points
    };

    struct Line {
        std::string text;
        std::string speech;
        std::vector <float> goal_face;
        std::vector <Choice> choices;
        int face_points; //reward for the right face
    };

    std::vector <Line> lines;

    //populates lines vector from file
    void loadDialogue (std::string filename);

    void loadTestDialogue ();

    std::vector <float> HAPPY;
    std::vector <float> SAD;

};
