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
    };

    struct Line {
        std::string text;
        std::vector <Choice> choices;
    };

    std::vector <Line> lines;

    //populates lines vector from file
    void loadDialogue (std::string filename);

    void loadTestDialogue ();

};
