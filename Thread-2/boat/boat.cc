#include <thread>
#include <vector>
#include <semaphore>
#include <unistd.h>

#include "boat.h"

namespace proj2{
int Oahu_child_counter = 0, Molokai_child_counter = 0, Oahu_adult_counter = 0;
bool isPilot = true;
bool isEnd = false;

std::binary_semaphore ThreadStart(0);
std::binary_semaphore ChildToMolokai(0);
std::binary_semaphore AdultToMolokai(0);
std::binary_semaphore ChildToOahu(0);
std::binary_semaphore End(0);
	
Boat::Boat(){
}

void Boat:: ChildThread(BoatGrader* bg){
    Oahu_child_counter++;
    bg->initializeChild();
    ThreadStart.release();

    while(true){
        ChildToMolokai.acquire();

        Oahu_child_counter--;

        if(isPilot){
            isPilot = false;
            bg->ChildRowToMolokai();
            ChildToMolokai.release();
        } else {
            if (Oahu_child_counter == 0 && Oahu_adult_counter == 0) isEnd = true;

            isPilot = true;
            bg->ChildRideToMolokai();
            Molokai_child_counter += 2;

            if (isEnd) {
                End.release();
            } else {
                ChildToOahu.release();
            }
        }

        ChildToOahu.acquire();
        Molokai_child_counter--;
        bg->ChildRowToOahu();
        Oahu_child_counter++;
        
        if (Oahu_child_counter >= 2) {
            ChildToMolokai.release();
        } else {
            AdultToMolokai.release();
        }
    }
}

void Boat:: AdultThread(BoatGrader* bg){
    Oahu_adult_counter++;
    bg->initializeAdult();
    ThreadStart.release();

    AdultToMolokai.acquire();
    Oahu_adult_counter--;
    bg->AdultRowToMolokai();
    ChildToOahu.release();
}

void Boat:: begin(int adults, int children, BoatGrader *bg){
    std::vector<std::thread> threadList;
    for (int i = 0; i < adults; i++) {
        threadList.push_back(std::thread(proj2::Boat::AdultThread, bg));
        ThreadStart.acquire();
    }
    for (int i = 0; i < children; i++) {
        threadList.push_back(std::thread(proj2::Boat::ChildThread, bg));
        ThreadStart.acquire();
    }

    ChildToMolokai.release();
    // ChildToMolokai.release();
    End.acquire();

    for (int i = 0; i < threadList.size(); i++) {
        threadList[i].detach();
    }
} 
}