#include "../include/mWindow.hpp"

int main()
{
    Window window(800, 600, L"Test Window");
    UserInput userInput;
    while(window.pullEvents(userInput));
    return 0;
}