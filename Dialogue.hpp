#pragma once

class Dialogue {
    public:
        Dialogue();
        ~Dialogue();

    struct Choice {
        string text;
        uint32_t goto_index; //index into Lines vector
    };

    struct Line {
        string text;
        std::vector <Choice> choices;
    };

    std::vector <Line> lines;

    //populates lines vector from file
    void loadDialogue (string filename);

    void loadTestDialogue ();

};
