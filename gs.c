#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// the board is modeled as an array of board_size ints
//   the i-th item of the array indicates the row of the i-th column occupied
typedef int * Queens;

// board_size is 8 for a normal chess board
const int board_size=8;

const int population_size=6; // must be even for some stuff to work (impl issue)!!

// for debugging each step
//#define DEBUGSTEP 1

#define COST(X) X[board_size]

#define VALMAX 30

#define MUTPROB 25

// we can define a small(16) or a big neighbourhood(56)
#define N16 1
#ifndef N16
#define N56
#endif


// We can define a probability of taking a solution which is not better
#define PROBBAD 30

// We can reset to the best of the previous batch of tries
#define RESET_BEST

// This is the limit of the number of iterations
#define NTRIES 50000
#define RECALLTRIES 100

// This is to set the initial state to be a random  configuration
#define RANDINIT

// Cost is number of instances where a queen is attacking another queen in a 
// column following it
int find_cost(Queens board){
	if ( board == NULL ) {
		printf("Invlid board, exiting !**\n");
		exit(1);
	}
	int cost = VALMAX;	// CHANGE: cost=VALMAX is no conflict
	for(int i=0;i<board_size;i++) 
		for(int j=i+1;j<board_size;j++) {
			if(board[i] == board[j])
				cost--;
			else if ( j-i == abs(board[i]-board[j]) )
					cost--;
		}
	return cost;
}


void sort_population(Queens boards[]){
	void copy_board(Queens, Queens);
	int temp[100]; // some large temporary space
	for(int i=0;i<population_size-1;i++)
		for(int j=i;j<population_size-1;j++)
			if(COST(boards[j]) < COST(boards[j+1])) {
				copy_board(boards[j],temp);
				copy_board(boards[j+1],boards[j]);
				copy_board(temp,boards[j+1]);
			}
}

// initial empty boards set
void initial_empty_boards(Queens boards[]){
	for(int p=0;p<population_size;p++)
		boards[p]=malloc((1+board_size)*sizeof(int));
}

// initial configuration
void initial_population(Queens boards[]){
	for(int p=0;p<population_size;p++) {
		boards[p]=malloc((1+board_size)*sizeof(int));
		for(int i=0;i<board_size;i++)
			boards[p][i]=rand()%8;  // choose random row for i-th column
		COST(boards[p])= find_cost(boards[p]);
	}
	// now sort it by cost
	sort_population(boards);
}
void show_solution(Queens board){
	for(int i=0;i<board_size;i++)
		printf("%d ",board[i]);
	printf("  ** %d\n",VALMAX-board[board_size]);
}

void copy_board(Queens board1, Queens board2){
	for(int i=0;i<board_size+1;i++) // +1 here because of COST
		board2[i]=board1[i];
}

void show_population(Queens boards[]){
	printf("--\n");
	for(int i=0;i<population_size;i++)
		show_solution(boards[i]);
}

int random_weighted_member(float frac[]){
	// noew generate a random number in in 0..1 and see which interval
	// it belongs and select that for the new population
	float r = rand()%100 / 100.0;
	if ( r <= frac[0]  )
		return 0;
	if ( r > frac[population_size-2])
		return population_size-1;
	for(int p=0;p<population_size-2;p++){
		if ( r > frac[p]  && r <= frac[p+1]){
			return p+1;
			break;
		}
	}
	printf("ERROR***\n");
	exit(1);
}

void mutate(Queens board){
	// find mutation point
	int c = rand()%8 ;
	// find mutated value
	board[c] = (board[c]+rand()%(board_size-1) + 1)%board_size;
}
void new_population(Queens boards[]){
	Queens old_boards[population_size];
	initial_empty_boards(old_boards);
	// copy the old board
	for(int p=0;p<population_size;p++)
		copy_board(boards[p],old_boards[p]);
	// find the sum of the costs
	int pop_cost=0;
	for(int p=0;p<population_size;p++)
		pop_cost+=COST(boards[p]);
	float frac[population_size]; // cum fraction of pop_cost
	for(int p=0;p<population_size;p++) {
		float f = COST(boards[p])/(float)pop_cost;
		if(p == population_size-1) {
			frac[p]=1;
			break;
		}
		frac[p]= (p==0?f:f+frac[p-1]);
	}

	// noew generate a random number in in 0..1 and see which interval
	// it belongs and select that for the new population
	for(int p=0;p<population_size;p+=2) {
		// select two members by weight probability from old populaiton
		int p1, p2;
		p1 = random_weighted_member(frac);
		while( (p2=random_weighted_member(frac)) == p1 ) ;

		// copy them after cross over to (new) boards
		int cp=rand()%7; // cross over point upto cp and after cp
		for(int i=0;i<=cp;i++) {
			boards[p][i]=old_boards[p2][i];
			boards[p+1][i]=old_boards[p1][i];
		}
		for(int i=cp+1;i<board_size;i++) {
			boards[p][i]=old_boards[p1][i];
			boards[p+1][i]=old_boards[p2][i];
		}
		COST(boards[p])= find_cost(boards[p]);
		COST(boards[p+1])= find_cost(boards[p+1]);
		// with a small probability also do a mutation in each
		if (rand()%100 < MUTPROB )
			mutate( boards[p]);
		if (rand()%100 < MUTPROB )
			mutate( boards[p+1]);
			
	}
}

int find_best_member(Queens boards[]){
	// we are maximizing
	int best=0;
	for(int p=1; p<population_size; p++){
		if (COST(boards[p]) > COST(boards[best]))
			best = p;
	}
	return best;
}
int find_worst_member(Queens boards[]){
	// we are maximizing
	int worst=0;
	for(int p=1; p<population_size; p++){
		if (COST(boards[p]) < COST(boards[worst]))
			worst = p;

	}
	return worst;
}

int main(){
	srand(getpid());  // otherwise the RNG gives the same sequence

	// initial guess
	Queens boards[population_size];
	initial_population(boards);
	printf("Intially...\n");
	show_population(boards);
#ifdef DEBUGSTEP
	getchar();
#endif

	int best_saved[100];
	int b = find_best_member(boards);
	copy_board(boards[b],best_saved);

	// In a loop keep looking for new populations
	int count=1;
	while(  count < NTRIES ) { // also check for solution found

		// Move to a new population
		new_population(boards);
		show_population(boards);

#ifdef DEBUGSTEP
		// for debug just wait before next iteration
		getchar();
#endif
		int b= find_best_member(boards);
		if (COST(boards[b]) > COST(best_saved))
			copy_board(boards[b],best_saved);

		if (COST(best_saved) == VALMAX)
			break;

		if ( count % RECALLTRIES == 0 ) {
			// randomly replace some item by the best saved
			int w=find_worst_member(boards);
			if ( COST(best_saved) > COST(boards[w]) )
				copy_board(best_saved,boards[w]);
		}

		
		count++;
	}

	// At this point display the best solution so far
	show_population(boards);
	printf("DONE at %d\n",count);
	show_solution(best_saved);
}
