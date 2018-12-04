#include "Dialogue.hpp"

//static bool DEBUG = true;

Dialogue::Dialogue() {
    loadTestDialogue(); //change this fn call to change what dialogue to load

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
    //TODO: change the text to something nice n displayable
    Line A;
    Line B;
    Line C;
    Line D;
    Line E;
    Line F;
    A.text = "A: Goal: Happy";
    A.goal_face = HAPPY;
    A.face_points = 50;
    B.text = "B: Goal: Sad";
    B.goal_face = SAD;
    B.face_points = 50;

    C.text = "C: Goal: Happy";
    C.goal_face = HAPPY;
    C.face_points = 50;
    D.text = "D: Goal: Sad";
    D.goal_face = SAD;
    D.face_points = 50;

    E.text = "E: Goal: Happy";
    E.goal_face = HAPPY;
    E.face_points = 50;
    F.text = "F: Goal: Sad";
    F.goal_face = SAD;
    F.face_points = 50;

    lines.emplace_back(A);
    lines.emplace_back(B);
    lines.emplace_back(C);
    lines.emplace_back(D);
    lines.emplace_back(E);
    lines.emplace_back(F);
}

void Dialogue::loadDateDialogue() {

}

Dialogue::~Dialogue() {
    //TODO
}
