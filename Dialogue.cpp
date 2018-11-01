#include "Dialogue.hpp"

static bool DEBUG = true;

Dialogue::Dialogue() {

}

void loadDialogue(string filename) {
    //TODO parse dialog from a file
}

void loadTestDialogue() {
    Line first;
    Line outcomeA;
    Line outcomeB;
    outcomeA.text = "Outcome A";
    outcomeB.text = "Outcome B";
    first.text = "Hi";

    lines.emplace_back(first);
    lines.emplace_back(outcomeA);
    lines.emplace_back(outcomeB);

    Choice c1 = {"Do this", 1};
    Choice c2 = {"Or that", 2};
    first.choices.emplace_back(c1);
    first.choices.emplace_back(c2);
}

Dialogue::~Dialogue() {
    //TODO
}
