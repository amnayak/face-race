#include "DialoguePlayer.hpp"

//static bool DEBUG = true;
static uint32_t NO_CHOICE = -1;

DialoguePlayer::DialoguePlayer (Dialogue d) {
    dialogue = d;
    line_number = -1;
    choice_index = NO_CHOICE;
    outstanding_choice = false;
}

//TODO this can also return a Line object?
//TODO makeChoice can be modified to use other state info
//TODO this is not at all resilient to bad input please take care
Dialogue::Line DialoguePlayer::playDialogue() {
    if (choice_index != NO_CHOICE && outstanding_choice) {
        line_number = dialogue.lines[line_number].choices[choice_index].goto_index;
        outstanding_choice = (dialogue.lines[line_number].choices.size() > 0) ? true : false;
        choice_index = NO_CHOICE;
    } else if (choice_index == NO_CHOICE && outstanding_choice) {
        //don't do anything
    } else {
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

DialoguePlayer::~DialoguePlayer() {
    //TODO
}
