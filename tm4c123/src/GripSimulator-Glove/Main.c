// Main.c

#include "Glove.h"
#include "Model.h"

#define GLOVE
// #define MODEL

int main(void) {

#ifdef GLOVE
    return main_glove();
#endif

#ifdef MODEL
    return main_model();
#endif

}

