#include <stdio.h>
#include "solver.h"
#include "API.h"

// You do not need to edit this file.
// This program just runs your solver and passes the choices
// to the simulator.
int main(int argc, char* argv[]) {
    debug_log("Running...");
    
    // Log the starting and goal positions for debugging
    char start_msg[50];
    char goal_msg[50];
    sprintf(start_msg, "Starting position: (%d, %d)", START_X, START_Y);
    sprintf(goal_msg, "Goal position: (%d, %d)", GOAL_X, GOAL_Y);
    debug_log(start_msg);
    debug_log(goal_msg);
    
    initialize();
    
    while (1) {
        Action nextMove = solver();

        // displays distances on the squares in the simulator
        for (int x = 0; x < MAZE_SIZE; ++x) {
            for (int y = 0; y < MAZE_SIZE; ++y) {
                API_setText(x, y, distances[x][y]);
            }
        }
        
        // Highlight the starting position in green
        API_setColor(START_X, START_Y, 'g');
        
        // Highlight the goal position in red
        API_setColor(GOAL_X, GOAL_Y, 'r');
        
        switch(nextMove){
            case FORWARD:
                API_moveForward();
                break;
            case LEFT:
                API_turnLeft();
                break;
            case RIGHT:
                API_turnRight();
                break;
            case IDLE:
                break;
        }
    }
}