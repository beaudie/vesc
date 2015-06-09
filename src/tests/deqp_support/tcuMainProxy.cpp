// Hack: we want to run some code before dEQP's main function the problem is that when we
// define our own main function there is a symbol conflict that is annoying to fix for all
// platforms. Instead of doing that, we just roll our own tcuMain that declares a deqp_main
// function instead of main.
#define main deqp_main
#include "tcuMain.cpp"
