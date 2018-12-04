#include "DialoguePlayer.hpp"

//static bool DEBUG = true;

DialoguePlayer::DialoguePlayer (Dialogue d) {
    dialogue = d;
    line_number = -1;
    choice_index = NO_CHOICE;
    outstanding_choice = false;
}

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

    //if you're at the last dialogue, signal gameOver
    if (line_number == dialogue.lines.size() - 1) {
      gameOver = true;
    }

    assert (line_number >= 0 && line_number < dialogue.lines.size());
    return dialogue.lines[line_number];
}

bool DialoguePlayer::makeChoice(uint32_t c) {
    if (outstanding_choice) {
        choice_index = c;
        return true;
    }
    return false;
}

bool DialoguePlayer::matchesGoalFace () {
  std::vector <float> goal_weights = dialogue.lines[line_number].goal_face;

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
