#include <iostream>
#include "sudoku.h"

using namespace std;

int main()
{
    Grid game;

    while (game.gameIsRunning())
    {
        system("clear");
        cout << game << '\n';
        cin >> game;
    }

    system("clear");
    cout << game << "\nYou Win!\n";

    return 0;
}