#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Parameters to play around with:
// NTRIES : number of iterations
// RANDINIT: This is to set the initial state to be a random  configuration
// 	If not defined then some (bad) initial configuration is chosen
// RESETBEST : define with to force resetting to a 'so-far-best' solution every so often
// N16 vs N56 : neighbourhood size
// PROBBAD : Probability of moving to a worse than current solution
// SATYPE : we can decide if PROBBAD is a fixed value or changes with iteration count
// DEBUGSTEP define this to wait for a return after each solution
// VERBOSE   define this to see each solution


// ---------------------------- INITIAL SETUPS
// for debugging each step
//#define DEBUGSTEP

// Default output is given only during reset (if enabled) and final cost
// for verbose output define VERBOSE
// #define VERBOSE

// This is the limit of the number of iterations
#define NTRIES 10000

// Determines initial configuration
#define RANDINIT

// We can reset to the best of the previous batch of tries
#define RESET_BEST

// we can define a small(16) or a big neighbourhood(56)
// #define N16 1   // comment this line for 56 neibourhood
#ifndef N16
#define N56
#endif

// We can define a probability of taking a solution which is not better
//#define SATYPE 1
#ifdef SATYPE
#define PROBBAD (60/(float)count)
#else
#define PROBBAD  30
#endif

// board_size is 8 for a normal chess board
const int board_size=8;
// the board is modeled as an array of board_size ints
//   the i-th item of the array indicates the row of the i-th column occupied
typedef int * Queens;

// Cost is number of instances where a queen is attacking another queen in a 
// column following it
int find_cost(Queens board){
	if ( board == NULL ) {
		printf("Invlid board, exiting !**\n");
		exit(1);
	}
	int cost = 0;
	for(int i=0;i<board_size;i++) 
		for(int j=i+1;j<board_size;j++) {
			if(board[i] == board[j])
				cost++;
			else if ( j-i == abs(board[i]-board[j]) )
					cost++;
		}
	return cost;
}

int compute_hash(Queens board){
	int sum=0;
	for(int i=0;i<board_size;i++)
		sum+=sum*10+board[i]%76543;
	return sum;
}

// This function defined how to move to a neighbour
int move_to_neighbour(Queens board, int count){
	int rc=random()%board_size;

	// compute howmuch to displace the queen
#ifdef N16
	int disp=random()%2+1;  // displace by adding 1 or 2
#endif
#ifdef N56
	int disp=random()%7+1; // displace by adding 1,2,.. or 7
#endif
	// remember old position and old cost
	int oldr=board[rc];
	int old_cost=find_cost(board);
	
	// set new position and get new cost
	board[rc]=(board[rc]+disp)%board_size; 
	int new_cost=find_cost(board);

	// decide if we move to this new position or not and return
	if (new_cost < old_cost )  		//  definitely move to new
		return new_cost;
	else {
#ifdef PROBBAD
	        if( random()%100 < PROBBAD)	// move to new with a prob
			return new_cost;
#endif
		board[rc]=oldr;
		return old_cost;
	}
}


// initial configuration
int initial_config(Queens * boardp){
	*boardp=malloc(board_size*sizeof(int));
	for(int i=0;i<board_size;i++)
#ifdef RANDINIT
		(*boardp)[i]=rand()%8;  // choose random row for i-th column
#else
		(*boardp)[i]=i;		// choose row i for i-th column
#endif
	return find_cost(*boardp);
}
void show_solution(Queens board){
	printf("--\n");
	for(int i=0;i<board_size;i++)
		printf("%d ",board[i]);
	printf("\n");
}

void copy_board(Queens board1, Queens board2){
	for(int i=0;i<board_size;i++)
		board2[i]=board1[i];
}

int main(){  
	// If you dont do this, rand() always gives the same sequence
	srand(getpid());

	// initial guess
	Queens board=NULL;
	int cost = initial_config(&board);
	show_solution(board);
	printf("Initial cost is %d\n",cost);

	// 1st one is the best solution so far, so store it
	Queens best_board=NULL;
	initial_config(&best_board);
	copy_board(best_board,board);
	int best_cost=cost;

	// In a loop keep looking for neighbours to go to
	int count=0;
	while( cost != 0 && count < NTRIES ) {

		// Move to a neighbour
		cost = move_to_neighbour(board,count);
#ifdef VERBOSE
		// display the new solution and its cost
		show_solution(board);
		printf("	cost is %d\n",cost);
#endif

		// remember it if it is a better solution
		if (cost <= best_cost) {   // we could try <= or <
			copy_board(board,best_board);
			best_cost=cost;
		}

#ifdef DEBUGSTEP
		// for debug just wait before next iteration
		getchar();
#endif
		
#ifdef RESET_BEST
		// Reset to remembered best once in a while OPTIONAL!
		if ( count%500 == 0 ){
			copy_board(best_board,board);
			cost=best_cost;
			printf("Reset cost: %d  hash: %d\n",best_cost,
							compute_hash(board));
		}
#endif
		count++;
	}

	// At this point display the best solution so far
	show_solution(best_board);
	printf("cost=%d\n",best_cost);
}
