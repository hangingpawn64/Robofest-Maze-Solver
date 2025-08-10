#include "solver.h"
#include "API.h"
#include "queue.h"

unsigned int maze[MAZE_SIZE][MAZE_SIZE] = { 0 };
int distances[MAZE_SIZE][MAZE_SIZE] = { -1 };   // -1 if it hasn't been visited yet
struct Coordinate position;
Heading heading;

int reached_goal = 0;     // "boolean" that stores whether the mouse should start exploring more squares

void initialize() {
    // Initialize maze array to all zeros
    for (int x = 0; x < MAZE_SIZE; ++x) {
        for (int y = 0; y < MAZE_SIZE; ++y) {
            maze[x][y] = 0;
        }
    }
    
    // setting the borders - only set walls that are actually on the border
    for (int i = 0; i < MAZE_SIZE; ++i) {
        maze[i][0] |= _0010;              // south wall for bottom row
        maze[i][MAZE_SIZE - 1] |= _1000;  // north wall for top row
        maze[0][i] |= _0001;              // west wall for left column
        maze[MAZE_SIZE - 1][i] |= _0100;  // east wall for right column
    }

    // setting initial distances
    resetDistances();

    // setting mouse position + heading to configurable starting position
    position.x = START_X;
    position.y = START_Y;
    heading = NORTH;  // You can change this if needed
}

/*
Updates the maze's walls based on what the mouse can currently see
*/
void updateMaze() {
    int x = position.x;
    int y = position.y;
    // start by assuming there are no walls, this variable will be changed based on which walls you see 
    unsigned int walls = _0000;

    switch (heading) {
        case NORTH:
            if (API_wallFront()) {
                walls |= _1000; // stores the wall to the north in walls (to be updated at the end of switch statement)
                // updating neighboring squares as well (if there is one):
                if (y + 1 < MAZE_SIZE)
                    maze[x][y + 1] |= _0010;
            }
            if (API_wallLeft()) {
                walls |= _0001;
                if (x - 1 >= 0)
                    maze[x - 1][y] |= _0100;
            }
            if (API_wallRight()) {
                walls |= _0100;
                if (x + 1 < MAZE_SIZE)
                    maze[x + 1][y] |= _0001;
            }
            break;
        case EAST:
            if (API_wallFront()) {
                walls |= _0100;
                if (x + 1 < MAZE_SIZE)
                    maze[x + 1][y] |= _0001;
            }
            if (API_wallLeft()) {
                walls |= _1000;
                if (y + 1 < MAZE_SIZE)
                    maze[x][y + 1] |= _0010;
            }
            if (API_wallRight()) {
                walls |= _0010;
                if (y - 1 >= 0)
                    maze[x][y - 1] |= _1000;
            }
            break;
        case SOUTH:
            if (API_wallFront()) {
                walls |= _0010;
                if (y - 1 >= 0)
                    maze[x][y - 1] |= _1000;
            }
            if (API_wallLeft()) {
                walls |= _0100;
                if (x + 1 < MAZE_SIZE)
                    maze[x + 1][y] |= _0001;
            }
            if (API_wallRight()) {
                walls |= _0001;
                if (x - 1 >= 0)
                    maze[x - 1][y] |= _0100;
            }
            break;
        case WEST:
            if (API_wallFront()) {
                walls |= _0001;
                if (x - 1 >= 0)
                    maze[x - 1][y] |= _0100;
            }
            if (API_wallLeft()) {
                walls |= _0010;
                if (y - 1 >= 0)
                    maze[x][y - 1] |= _1000;
            }
            if (API_wallRight()) {
                walls |= _1000;
                if (y + 1 < MAZE_SIZE)
                    maze[x][y + 1] |= _0010;
            }
            break;
    }

    maze[x][y] |= walls;

    // Update walls in simulator for visualization
    if (maze[x][y] >= 8) {
        API_setWall(x, y, 'n');
    }
    if (maze[x][y] % 8 >= 4) {
        API_setWall(x, y, 'e');
    }
    if (maze[x][y] % 4 >= 2) {
        API_setWall(x, y, 's');
    }
    if (maze[x][y] % 2 == 1) {
        API_setWall(x, y, 'w');
    }
}

int xyToSquare(int x, int y) {
    return x + MAZE_SIZE * y;
}

struct Coordinate squareToCoord(int square) {
    struct Coordinate coord;
    coord.x = square % MAZE_SIZE;
    coord.y = square / MAZE_SIZE;
    return coord;
}

void resetDistances() {
    // initially sets all the distances to -1 (invalid distance)
    for (int x = 0; x < MAZE_SIZE; ++x) {
        for (int y = 0; y < MAZE_SIZE; ++y) {
            distances[x][y] = -1;
        }
    }

    // if you haven't reached the goal, set the goal to be the target position
    if (!reached_goal) {
        distances[GOAL_X][GOAL_Y] = 0; 
    }
    else {
        distances[START_X][START_Y] = 0;    // go back to the start
    }
}

int isWallInDirection(int x, int y, Heading direction) {
    switch (direction) {
        case NORTH:
            if (maze[x][y] >= 8)
                return 1;
            break;
        case EAST:
            if (maze[x][y] % 8 >= 4)
                return 1;
            break;
        case SOUTH:
            if (maze[x][y] % 4 >= 2)
                return 1;
            break;
        case WEST:
            if (maze[x][y] % 2 == 1) 
                return 1;
            break;
    }
    return 0;
}

void updateDistances() {
    resetDistances();
    queue squares = queue_create();

    // adds the goal squares to the queue (the target position or the starting position depending on if you've reached the goal)
    for (int x = 0; x < MAZE_SIZE; ++x) {
        for (int y = 0; y < MAZE_SIZE; ++y) {
            if (distances[x][y] == 0)
                queue_push(squares, xyToSquare(x, y));
        }
    }

    while (!queue_is_empty(squares)) {
        struct Coordinate square = squareToCoord(queue_pop(squares));
        int x = square.x;
        int y = square.y;

        // if there's no wall to the north && the square to the north hasn't been checked yet
        if (y + 1 < MAZE_SIZE && isWallInDirection(x, y, NORTH) == 0 && distances[x][y + 1] == -1) {
            distances[x][y + 1] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x, y + 1));
        }
        // same as ^ but for east
        if (x + 1 < MAZE_SIZE && isWallInDirection(x, y, EAST) == 0 && distances[x + 1][y] == -1) {
            distances[x + 1][y] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x + 1, y));
        }
        // same as ^ but for south
        if (y - 1 >= 0 && isWallInDirection(x, y, SOUTH) == 0 && distances[x][y - 1] == -1) {
            distances[x][y - 1] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x, y - 1));
        }
        // same as ^ but for west
        if (x - 1 >= 0 && isWallInDirection(x, y, WEST) == 0 && distances[x - 1][y] == -1) {
            distances[x - 1][y] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x - 1, y));
        }
    }
    
    queue_destroy(squares);
}

void updateHeading(Action nextAction) {
    if (nextAction == FORWARD || nextAction == IDLE) {
        return;
    }
    else if (nextAction == LEFT) {
        switch (heading) {
            case NORTH:
                heading = WEST;
                break;
            case EAST:
                heading = NORTH;
                break;
            case SOUTH:
                heading = EAST;
                break;
            case WEST:
                heading = SOUTH;
                break;
            default:
                break;
        }
    }
    else if (nextAction == RIGHT) {
        switch (heading) {
            case NORTH:
                heading = EAST;
                break;
            case EAST:
                heading = SOUTH;
                break;
            case SOUTH:
                heading = WEST;
                break;
            case WEST:
                heading = NORTH;
                break;
            default:
                break;
        }
    }
}

void updatePosition(Action nextAction) {
    if (nextAction != FORWARD) {
        return;
    }

    switch (heading) {
        case NORTH:
            position.y += 1;
            break;
        case SOUTH:
            position.y -= 1;
            break;
        case EAST:
            position.x += 1;
            break;
        case WEST:
            position.x -= 1;
            break;
        default:
            break;
    }
}

Action solver() {
    // if you reached the goal, go back to the start
    if (!reached_goal && position.x == GOAL_X && position.y == GOAL_Y) {
        reached_goal = 1;
    }
    // if you went to the goal & all the way back to the start, restart
    else if (reached_goal && position.x == START_X && position.y == START_Y) {
        reached_goal = 0;
    }

    updateMaze();
    updateDistances();

    Action action = floodFill();

    updateHeading(action);
    updatePosition(action);
    return action;
}

// Put your implementation of floodfill here!
Action floodFill() {
    unsigned int least_distance = 300;   // just some large number, none of the distances will be over 300
    Action optimal_move = IDLE;

    /*
    Basic Idea:
    - Look at the square in front of you, to the left, and to the right if there are no walls
    - Find the square with the lowest distance to the goal
    - Move in the direction of that square (move forward if it's the forward square, turn in the correct direction otherwise)
    */

    int x = position.x;
    int y = position.y;

    if (heading == NORTH) {
        if (y + 1 < MAZE_SIZE && !isWallInDirection(x, y, NORTH) && distances[x][y + 1] < least_distance) {
            least_distance = distances[x][y + 1];
            optimal_move = FORWARD;
        }
        if (x + 1 < MAZE_SIZE && !isWallInDirection(x, y, EAST) && distances[x + 1][y] < least_distance) {
            least_distance = distances[x + 1][y];
            optimal_move = RIGHT;
        }
        if (x - 1 >= 0 && !isWallInDirection(x, y, WEST) && distances[x - 1][y] < least_distance) {
            least_distance = distances[x - 1][y];
            optimal_move = LEFT;
        }
    }
    else if (heading == EAST) {
        if (x + 1 < MAZE_SIZE && !isWallInDirection(x, y, EAST) && distances[x + 1][y] < least_distance) {
            least_distance = distances[x + 1][y];
            optimal_move = FORWARD;
        }
        if (y - 1 >= 0 && !isWallInDirection(x, y, SOUTH) && distances[x][y - 1] < least_distance) {
            least_distance = distances[x][y - 1];
            optimal_move = RIGHT;
        }
        if (y + 1 < MAZE_SIZE && !isWallInDirection(x, y, NORTH) && distances[x][y + 1] < least_distance) {
            least_distance = distances[x][y + 1];
            optimal_move = LEFT;
        }
    }
    else if (heading == SOUTH) {
        if (y - 1 >= 0 && !isWallInDirection(x, y, SOUTH) && distances[x][y - 1] < least_distance) {
            least_distance = distances[x][y - 1];
            optimal_move = FORWARD;
        }
        if (x - 1 >= 0 && !isWallInDirection(x, y, WEST) && distances[x - 1][y] < least_distance) {
            least_distance = distances[x - 1][y];
            optimal_move = RIGHT;
        }
        if (x + 1 < MAZE_SIZE && !isWallInDirection(x, y, EAST) && distances[x + 1][y] < least_distance) {
            least_distance = distances[x + 1][y];
            optimal_move = LEFT;
        }
    }
    else if (heading == WEST) {
        if (x - 1 >= 0 && !isWallInDirection(x, y, WEST) && distances[x - 1][y] < least_distance) {
            least_distance = distances[x - 1][y];
            optimal_move = FORWARD;
        }
        if (y + 1 < MAZE_SIZE && !isWallInDirection(x, y, NORTH) && distances[x][y + 1] < least_distance) {
            least_distance = distances[x][y + 1];
            optimal_move = RIGHT;
        }
        if (y - 1 >= 0 && !isWallInDirection(x, y, SOUTH) && distances[x][y - 1] < least_distance) {
            least_distance = distances[x][y - 1];
            optimal_move = LEFT;
        }
    }

    // handles dead ends (when there's no walls in front, to the right or to the left)
    if (least_distance == 300)
        optimal_move = RIGHT;   // arbitrary, can be any turn
    
    return optimal_move;
}

// This is an example of a simple left wall following algorithm.
Action leftWallFollower() {
    if(API_wallFront()) {
        if(API_wallLeft()){
            return RIGHT;
        }
        return LEFT;
    }
    return FORWARD;
}