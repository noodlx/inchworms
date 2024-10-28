#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <ncurses.h>
#include <time.h>

//global variables and mutex
bool ready[4] = {true, true, true, true};
char quit;
pthread_mutex_t lock;

//screen updating thread that checks if all threads are ready before each update and also checks for user 'quit input'
void *updateScr()
{
    noecho();
    nodelay(stdscr, true);
    while (quit != 'q')
    {
        if( !ready[0] && !ready[1] && !ready[2] && !ready[3])
        {    
            mvaddstr(6, 12, "Press 'Q' to Exit.");
            refresh();
            for( int i = 0; i < 4; i++)
                ready[i] = true;
        }  
        quit = getch();
        usleep(15000);
    }
}

// checks if the worm is within 5 coordinates of the ceiling or floor
// returns 0 if coordinate is not too close to a y value
// returns 1 if coordinate is too close to top
// returns 2 if coordinate is too close to bottom
int tooCloseY( int i, int yMax)
{
    int distance = yMax - i;
    if( i <= 5)
    {
        return 1;
    }
    else if(distance <= 5)
    {
        return 2;
    }
    else return 0;
}


// checks if the worm is within 5 coordinates wall
// returns 0 if coordinate is not too close to a x value
// returns 1 if coordinate is too close to left wall
// returns 2 if coordinate is too close to right wall
int tooCloseX( int i, int xMax )
{
    int distance = xMax - i;
    if( i <= 5)
    {
        return 1;
    }
    else if(distance <= 5)
    {
        return 2;
    }
    else return 0;
}

//returns true if the inchworm head is within columns or rows of a corner
bool closeCorner( int headYX[], int yMax, int xMax)
{
    if( (yMax - headYX[0] <= 7 && xMax - headYX[1] <= 7) ||
        (headYX[0] <= 7 && xMax - headYX[1] <= 7) ||
        (yMax - headYX[0] <= 7 && headYX[1] <= 7) ||
        (headYX[0] <= 7 && headYX[1] <= 7))
         return true; 
    else return false;
}

//updates the ncurses display
//uses a mutex to make sure that the inchworm threads don't overlead the stdscreen
//true = erase;
//false = draw;
void redisplay( bool state, int coords[][2], int thread)
{
    pthread_mutex_lock(&lock);
    if(state)
    {
        mvaddch(coords[3][0], coords[3][1], ' ');
        mvaddch(coords[4][0], coords[4][1], ' ');
    }
    else
    {
        mvaddch(coords[0][0], coords[0][1], '@');
        mvaddch(coords[1][0], coords[1][1], '#');
        mvaddch(coords[2][0], coords[2][1], '#');
    }
    ready[thread] = false;
    pthread_mutex_unlock(&lock);
}

//worm thread function
//takes an argument that becomes the thead number
void *wormFunc(void *arg)
{
    int direction = 0; 
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    int thread = arg;
    srand((unsigned int)thread);  
    int bodyYX[5][2] = {{yMax/2, (xMax/2)+thread}, {(yMax/2)+1, (xMax/2)+thread}, {(yMax/2)+2, (xMax/2)+thread},
                        {(yMax/2)+3, (xMax/2)+thread}, {(yMax/2)+4, (xMax/2)+thread}};
    
    //main loop that calculates the direction, then updates the local array that stores the coordinates of all sections of inchworm
    //this array is passed to the redisplay function to update the stdscreen
    //the loop ends when the quit flag is raised, which allows the thread to finish it's work.
    while( quit != 'q')
    {
        int yState = tooCloseY(bodyYX[0][0], yMax);
        int xState = tooCloseX(bodyYX[0][1], xMax);   
        int random = (int)(rand()%4);

        // with a corner distance of 7 and wall distance of 5, there is a rare case where a corner is approached
        // at a specific cross point of the wall-boundary and the corner-boundary
        // because of the stipulation in the instructions that at a corner the worms "turn right", this creates a loop in the corner
        // to remedy this i made the worms turn 90 degrees right (which they dont do anywhere else), but only at a corner boundary.
        if(closeCorner(bodyYX[0], yMax, xMax))
        {
            direction += 2;
            direction %= 8;
        }
        //checks if atleast 1 collission state is active
        else if(yState != 0 || xState != 0)
        {   
            //redirects the worm away from the ceiling in either the least number of turns
            //or when approached head on towards the further corner
            if( yState == 1)
            {
                if( direction == 1 || direction == 2)
                {
                    direction += 1;
                    direction %= 8;
                }
                else if (direction == 7 || direction == 6)
                {
                    direction -= 1;
                    direction %= 8;
                }
                else if (bodyYX[0][1] > xMax/2)
                {
                    direction -= 1;
                    direction %= 8;
                }  
                else
                {
                    direction += 1;
                    direction %= 8;
                }   
            }
            //redirects the worm away from the floor in either the least number of turns
            //or when approached head on towards the further corner
            else if( yState == 2)
            {
                if( direction == 2 || direction == 3)
                {
                    direction -= 1;
                    direction %= 8;
                }
                else if (direction == 5 || direction == 6)
                {
                    direction += 1;
                    direction %= 8;
                }
                else if (bodyYX[0][1] > xMax/2)
                {
                    direction += 1;
                    direction %= 8;
                }  
                else
                {
                    direction -= 1;
                    direction %= 8;
                }   
            } 
            //redirects the worm away from the left wall in either the least number of turns
            //or when approached head on towards the further corner
            else if( xState == 1)
            {
                if( direction == 4 || direction == 5 )
                {
                    direction -= 1;
                    direction %= 8;
                }
                else if (direction == 0 || direction == 7)
                {
                    direction += 1;
                    direction %= 8;
                }
                else if (bodyYX[0][0] > yMax/2)
                {
                    direction += 1;
                    direction %= 8;
                }  
                else
                {
                    direction -= 1;
                    direction %= 8;
                }   
            } 
            //redirects the worm away from the right wall in either the least number of turns
            //or when approached head on towards the further corner
            else if( xState == 2)
            {
                if( direction == 0 || direction == 1 )
                {
                    direction -= 1;
                    direction %= 8;
                }
                else if (direction == 3 || direction == 4)
                {
                    direction += 1;
                    direction %= 8;
                }
                else if (bodyYX[0][0] > yMax/2)
                {
                    direction -= 1;
                    direction %= 8;
                }  
                else
                {
                    direction += 1;
                    direction %= 8;
                }   
            }
        }
        // 1/4 random chance of turning right if no impending collision is detected
        else if(random == 0)
        {
                direction += 1;
                direction %= 8;
        }
        // 1/4 random chance of turning left if no impending collision is detected
        else if(random == 1)
        {
                direction -= 1;
                direction %= 8;
        }
      
        // calls the redisplay function to erase the back two segments of the inchworm
        // then enters an innocuous loop until all threads are ready to continue onto the calculation phase
        redisplay(true, bodyYX, thread);
        while(!ready[thread])
        {
            usleep(250000);
        }

        //rolls over the 8 directional system... ie: to turn left from direction 0 would be to turn to direction 7
        if( direction < 0)
        {
            direction = 7;
        }
        //calculates the new positions of all segments of the worm based on the 8 directional system
        //the calculation loop calculates starting at the back of the worm and finishes calculations with the head position.
        //the guidelines say to put this calculation in the redisplay, but there is no need for the erase redisplay to see the direction,
        //so i chose to streamline the redisplay function by placing the worm direction calculation in the worm thread directly.
        //furthermore, because I lock the redisplay to keep it thread-safe, I want to minimize the processing that needs to be done in the locked function.
        switch(direction)
        {
            case 0: //up
                for(int i = 4; i >= 0; i--)
                {    
                    //calculates 2nd segment
                     if( i == 1)
                     {
                        bodyYX[i][0] = bodyYX[2][0]-1;
                        bodyYX[i][1] = bodyYX[2][1];
                     }
                     //calculates head
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]-2;
                        bodyYX[i][1] = bodyYX[2][1];
                     }
                     //calculates back 3 segments
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 1: //up-right
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0]-1;
                        bodyYX[i][1] = bodyYX[2][1]+1;
                    }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]-2;
                        bodyYX[i][1] = bodyYX[2][1]+2;
                    }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 2: //right
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0];
                        bodyYX[i][1] = bodyYX[2][1]+1;
                     }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0];
                        bodyYX[i][1] = bodyYX[2][1]+2;
                     }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 3: //down-right
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+1;
                        bodyYX[i][1] = bodyYX[2][1]+1;
                    }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+2;
                        bodyYX[i][1] = bodyYX[2][1]+2;
                    }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 4: //down
                for(int i = 4; i >= 0; i--)
                {
                     if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+1;
                        bodyYX[i][1] = bodyYX[2][1];
                     }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+2;
                        bodyYX[i][1] = bodyYX[2][1];
                     }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 5: //down-left
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+1;
                        bodyYX[i][1] = bodyYX[2][1]-1;
                    }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]+2;
                        bodyYX[i][1] = bodyYX[2][1]-2;
                    }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 6: //left
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0];
                        bodyYX[i][1] = bodyYX[2][1]-1;
                     }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0];
                        bodyYX[i][1] = bodyYX[2][1]-2;
                    }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            case 7: //up-left
                for(int i = 4; i >= 0; i--)
                {
                    if( i == 1)
                    {
                        bodyYX[i][0] = bodyYX[2][0]-1;
                        bodyYX[i][1] = bodyYX[2][1]-1;
                    }
                    else if( i == 0)
                    {
                        bodyYX[i][0] = bodyYX[2][0]-2;
                        bodyYX[i][1] = bodyYX[2][1]-2;
                    }
                    else
                    {   
                        bodyYX[i][0] = bodyYX[i-2][0];
                        bodyYX[i][1] = bodyYX[i-2][1];
                    }
                }
                break;
            default:
                break;
        }

        // calls the redisplay with 'false' to signal the drawing of the new positions of the worm segments
        // enters an innocuous loop afterwards to wait for other threads to finish before the update
        // allows the thread to loop back to the top.
        redisplay(false, bodyYX, thread);
        while(!ready[thread])
        {
            usleep(250000);
        }
    }
}

int main()
{
//initiates the stdscreen
    initscr();

//creates the 4 inchworm threads, throwing an error if any loop fails
    int check;
    pthread_t a_thread;
    check = pthread_create(&a_thread, 0, wormFunc, (void*)0);
    if( check != 0)
    {
        perror("Worm Thread A Failed");
        exit(0);
    }
    pthread_t b_thread;
    check = pthread_create(&b_thread, 0, wormFunc, (void*)1);
    if( check != 0)
    {
        endwin();
        perror("Worm Thread B Failed");
        exit(0);
    }
    pthread_t c_thread;
    check = pthread_create(&c_thread, 0, wormFunc, (void*)2);
    if( check != 0)
    {
        endwin();
        perror("Worm Thread C Failed");
        exit(0);
    }
    pthread_t d_thread;
    check = pthread_create(&d_thread, 0, wormFunc, (void*)3);
    if( check != 0)
    {
        endwin();
        perror("Worm Thread D Failed");
        exit(0);
    }

    //creates the updater threadd
    pthread_t upd_thread;
    check = pthread_create(&upd_thread, 0, updateScr, (void*)0);
    if( check != 0)
    {
        endwin();
        perror("Worm Thread E Failed");
        exit(0);
    } 

    //checks every second if a quit has been called then ends the window and destroys the mutex before closing the program
    while( quit != 'q')
    {
       usleep(100000);
    }

    nodelay(stdscr, false);
    getch();
    endwin();
    pthread_mutex_destroy(&lock);
    return 0;    
}

