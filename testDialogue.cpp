#include <iostream>
#include "DialoguePlayer.hpp"
#include "Dialogue.hpp"

int main() {
    std::cout << "++++++++++++++++++++++++";
    Dialogue d = Dialogue();
    DialoguePlayer dp = DialoguePlayer(d);
    std::cout << dp.playDialogue().text;
    dp.makeChoice(0);
    std::cout << dp.playDialogue().text;

    (void)dp;
    return 0;
}
