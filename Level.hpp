#pragma once

class Level {
    public:
        Level();
        ~Level();

    struct Expression {
        //weights?????????
    }

    struct Performance {
        //some way of tracking how well someone is doing
        //and their progress
    }

    std::vector <state> states;
    uint32_t current_state;

    struct state {
        Dialogue dialoguePlayer;
        Expression goalFace;
        Expression currentFace;
        Performance marker; //TODO
   };
};
