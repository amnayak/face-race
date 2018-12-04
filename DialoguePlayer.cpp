#include "DialoguePlayer.hpp"

//static bool DEBUG = true;
static uint32_t NO_CHOICE = -1;

DialoguePlayer::DialoguePlayer (Dialogue d) {
    dialogue = d;
    line_number = -1;
    choice_index = NO_CHOICE;
    outstanding_choice = false;
}

//TODO change function to:
//1) accrue points based on face challenge, choice challenge, or both
//2) go to next line properly and not crash
//3) set gameOver flag
Dialogue::Line DialoguePlayer::playDialogue() {
    if (choice_index != NO_CHOICE && outstanding_choice) {
        line_number = dialogue.lines[line_number].choices[choice_index].goto_index;
        outstanding_choice = (dialogue.lines[line_number].choices.size() > 0) ? true : false;
        choice_index = NO_CHOICE;
    } else if (choice_index == NO_CHOICE && outstanding_choice) {
        //don't do anything
    } else {
        //no outstanding choice to make
        line_number++;
        outstanding_choice = (dialogue.lines[line_number].choices.size() > 0) ? true : false;
        choice_index = NO_CHOICE;
    }

    assert (line_number >= 0);
    return dialogue.lines[line_number];
}

bool DialoguePlayer::makeChoice(uint32_t c) {
    if (outstanding_choice) {
        choice_index = c;
        return true;
    }
    return false;
}

bool DialoguePlayer::matchesGoalFace (std::vector <float> goal_weights, std::vector <float> curr_weights) {
    for (int i = 0; i < goal_weights.size(); i++) {
        if (curr_weights[i] < goal_weights[i] + threshold && curr_weights[i] > goal_weights[i] - threshold) {
            //yay
        } else {
            return false;
        }
    }
    player_points += dialogue.lines[line_number].face_points;
    return true;
}

DialoguePlayer::~DialoguePlayer() {
    //TODO
}
