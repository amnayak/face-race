#include "Dialogue.hpp"

//static bool DEBUG = true;

Dialogue::Dialogue() {
    loadTestDialogue();

    //TODO: initialize the hardcoded faces
}

void Dialogue::loadDialogue(std::string filename) {
    //TODO: parse dialog from a file
}

void Dialogue::loadTestDialogue() {
  /**
    Line first;
    Line outcomeA;
    Line outcomeB;
    outcomeA.text = "Outcome A";
    outcomeA.goalFace = HAPPY;
    outcomeA.facePoints = 50;
    outcomeB.text = "Outcome B";
    outcomeB.goalFace = SAD;
    outcomeB.facePoints = 50;
    first.text = "Hi";

    lines.emplace_back(first);
    lines.emplace_back(outcomeA);
    lines.emplace_back(outcomeB);

    Choice c1 = {"Do this", 1, 15};
    Choice c2 = {"Or that", 2, 20};
    first.choices.emplace_back(c1);
    first.choices.emplace_back(c2);
    **/
    Line A;
    Line B;
    outcomeA.text = "A";
    outcomeA.goalFace = HAPPY;
    outcomeA.facePoints = 50;
    outcomeB.text = "B";
    outcomeB.goalFace = SAD;
    outcomeB.facePoints = 50;

    lines.emplace_back(outcomeA);
    lines.emplace_back(outcomeB);
}

Dialogue::~Dialogue() {
    //TODO
}
