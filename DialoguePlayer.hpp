#pragma once
#include "Dialogue.hpp"

class DialoguePlayer {
    public:
        DialoguePlayer(Dialogue d);
        ~DialoguePlayer();

        //TODO store all your other metrics in here

    //dialogue to walk through
    Dialogue dialogue;

    uint32_t line_number; //line of dialogue you are on,
                     //indexes into dialogue.lines
    uint32_t choice_index; //choice player chooses within the last
                     //played dialogue
    bool outstanding_choice; //current line has an outstanding
                             //choice

    //Returns the next line to display. If there is no
    //outstanding choice to be made, increment and play next
    //line. If there is one, play the same dialogue.
    //If player has made a choice, jump to correct line.
    Dialogue::Line playDialogue();

    //inputs player choice
    bool makeChoice(uint32_t c);

    //TODO: set hese to real values
    std::vector <float> HAPPY;
    std::vector <float> SAD;

    float threshold = 30.f;

};
