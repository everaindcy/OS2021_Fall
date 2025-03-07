#ifndef BOAT_H_
#define BOAT_H_

#include<stdio.h>
#include <thread>
#include <mutex>
#include <semaphore>
#include <unistd.h>

#include "boatGrader.h"

namespace proj2{
class Boat{
public:
	Boat();
    ~Boat(){};
	void begin(int, int, BoatGrader*);
	static void ChildThread(BoatGrader*);
	static void AdultThread(BoatGrader*);
};
}

#endif // BOAT_H_
