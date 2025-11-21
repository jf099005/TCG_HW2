#include"node.h"
#ifndef SIMULATOR
#define SIMULATOR 1

typedef int Score;

class CDC_simulator{
    public:
        CDC_simulator()
        {};
    
        Score simulate(Position pos);
};

#endif