#pragma once
#include "Dialogue.hpp"

class DialoguePlayer {
    public:
        DialoguePlayer(Dialogue d);
        ~DialoguePlayer();

    //tracks the number of points the player
    //has accrued
    int player_points;

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

    std::vector <float> curr_weights; //face weights used to judge user

    //inputs player choice, and changes score based on it
    bool makeChoice(uint32_t c);

    //set these to real values
    /**
     *  HAPPY:
        mouth_up L/R  = 1.0;
        brow_down L/R = 0.5;

        SAD:
        brow_down L/R = 1.0;
        mouth_down L/R = 1.0;
     * */
    //std::vector <float> HAPPY (length, 0.0);
    //std::vector <float> SAD (length, 0.0);

    //returns true if curr_weights matches goal_weights with a certain threshhold
    //updates player points
    bool matchesGoalFace ();

    bool gameOver = false;

    float threshold = 0.25f;

    uint32_t NO_CHOICE = -1;

};
