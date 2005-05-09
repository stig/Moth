#include <stdio.h>
#include <assert.h>
#include <ggtl.h>

/* prototype for callbacks */
bool end_of_game(const void *boarddata, int me);
nodeptr find_moves(const void *boarddata, nodeptr *availmoves, int me);
bool make_move(void *boarddata, const void *movedata, int me);
int evaluate(const void *boarddata, int me);

static bool valid_move(char *board, int x, int y, int me, bool domove);
void display(const void *boarddata);
void mainloop(struct ggtl *game);

#define a(A, B, C) A[(B) * 8 + (C)]

int main(void)
{
	struct ggtl *game;
	char board[8][8] = {{0}};

	board[3][3] = board[4][4] = 1;
	board[3][4] = board[4][3] = 2;

	game = ggtl_new(board, sizeof board, 2);
	ggtl_add_callbacks(game, end_of_game, find_moves, make_move, evaluate);

	mainloop(game);

	return 0;
}

void display(const void *boarddata)
{
	const char *board = boarddata;
	int i, j, c;

	printf("   ");
	for (i = 0; i < 8; i++)
		printf(" %d  ", i);
	puts("\n  +---+---+---+---+---+---+---+---+");

	for (i = 0; i < 8; i++) {
		printf("%d |", i);
		for (j = 0; j < 8; j++) {
			c = a(board, i, j);
			if (c != 0)
				printf(" %d |", c);
			else 
				printf("   |");
		}
		puts("\n  +---+---+---+---+---+---+---+---+");

	}
}

void mainloop(struct ggtl *game)
{	
	const void *board;

	while (ggtl_alphabeta(game, 2)) {
		board = ggtl_peek_current_state(game);
		display(board);
		puts("press enter for next move");
		getchar();
	}
}

static int eval_heuristic(const char *board, int me)
{
	int i, j, c, score = 0;
	int not_me = 3 - me;
	int heuristic[8][8] = { {9, 2, 7, 8, 8, 7, 2, 9},
				{2, 1, 3, 4, 4, 3, 1, 2},
				{7, 3, 6, 5, 5, 6, 3, 7},
				{8, 4, 5, 1, 1, 5, 4, 8},
				{8, 4, 5, 1, 1, 5, 4, 8},
				{7, 3, 6, 5, 5, 6, 3, 7},
				{2, 1, 3, 4, 4, 3, 1, 2},
				{9, 2, 7, 8, 8, 7, 2, 9} };

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = a(board, i, j);
			if (c == me) 
				score += heuristic[i][j];
			else if (c == not_me)
				score -= heuristic[i][j];
		}
	}
	return score;
}

int evaluate(const void *boarddata, int me)
{
	const char *board = boarddata;
	int score;
	
	score = eval_heuristic(board, me);
	return score;
}


nodeptr find_moves(const void *boarddata, nodeptr *availmoves, int me)
{
	nodeptr tmp, movelist = NULL;
	const char *board = boarddata;
	char *mv, i, j;
	
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me, false)) {
				tmp = sll_pop(availmoves); 
				assert(tmp != NULL); 
				mv = sll_peek(tmp); 
				mv[0] = i + '0'; 
				mv[1] = j + '0'; 
				sll_push(&movelist, tmp); 
			}
		}
	}
	return movelist;
}

bool end_of_game(const void *boarddata, int me)
{
	const char *board = boarddata;
	int i, j, not_me = 3 - me;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me, false))
				return false;
			if (valid_move(board, i, j, not_me, false))
				return false;
		}
	}
	return true;
}

bool make_move(void *boarddata, const void *movedata, int me)
{
	char *board = boarddata;
	const char *move = movedata;
	int x, y;
	
	assert(move != NULL);

	x = move[0] - '0';
	y = move[1] - '0';
	return valid_move(board, x, y, me, true);
}

static bool valid_move(char *board, int x, int y, int me, bool domove)
{
	int not_me = 3 - me;
	int tx, ty, flipped = 0;

	/* slot must not already be occupied */
	if (a(board, x, y) != 0)
		return false;

	/* left */
	for (tx = x - 1; tx >= 0 && a(board, tx, y) == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && a(board, tx, y) == me) {
		if (domove == false)
			return true;

		tx = x - 1;
		while (tx >= 0 && a(board, tx, y) == not_me) {
			a(board, tx, y) = me;
			tx--;
		}
		flipped++;
	}

	/* right */
	for (tx = x + 1; tx < 8 && a(board, tx, y) == not_me; tx++)
		;
	if (tx < 8 && tx != x + 1 && a(board, tx, y) == me) {
		if (domove == false)
			return true;

		tx = x + 1;
		while (tx < 8 && a(board, tx, y) == not_me) {
			a(board, tx, y) = me;
			tx++;
		}
		flipped++;
	}

	/* up */
	for (ty = y - 1; ty >= 0 && a(board, x, ty) == not_me; ty--)
		;
	if (ty >= 0 && ty != y - 1 && a(board, x, ty) == me) {
		if (domove == false)
			return true;

		ty = y - 1;
		while (ty >= 0 && a(board, x, ty) == not_me) {
			a(board, x, ty) = me;
			ty--;
		}
		flipped++;
	}
	
	/* down */
	for (ty = y + 1; ty < 8 && a(board, x, ty) == not_me; ty++)
		;
	if (ty < 8 && ty != y + 1 && a(board, x, ty) == me) {
		if (domove == false)
			return true;

		ty = y + 1;
		while (ty < 8 && a(board, x, ty) == not_me) {
			a(board, x, ty) = me;
			ty++;
		}
		flipped++;
	}
	
	/* up/left */
	tx = x - 1;
	ty = y - 1; 
	while (tx >= 0 && ty >= 0 && a(board, tx, ty) == not_me) {
		tx--; ty--;
	}
	if (tx >= 0 && ty >= 0 && ty != y - 1 && tx != x - 1 && 
			a(board, tx, ty) == me) {
		if (domove == false)
			return true;

		tx = x - 1;
		ty = y - 1;
		while (tx >= 0 && ty >= 0 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx--; ty--;
		}
		flipped++;
	}

	/* up/right */
	tx = x - 1;
	ty = y + 1; 
	while (tx >= 0 && ty < 8 && a(board, tx, ty) == not_me) {
		tx--; ty++;
	}
	if (tx >= 0 && ty < 8 && ty != y - 1 && tx != x + 1 && 
			a(board, tx, ty) == me) {
		if (domove == false)
			return true;

		tx = x - 1;
		ty = y + 1;
		while (tx >= 0 && ty < 8 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx--; ty++;
		}
		flipped++;
	}
	
	/* down/right */
	tx = x + 1;
	ty = y + 1; 
	while (tx < 8 && ty < 8 && a(board, tx, ty) == not_me) {
		tx++; ty++;
	}
	if (tx < 8 && ty < 8 && ty != y + 1 && tx != x + 1 && 
			a(board, tx, ty) == me) {
		if (domove == false)
			return true;

		tx = x + 1;
		ty = y + 1;
		while (tx < 8 && ty < 8 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx++; ty++;
		}
		flipped++;
	}

	/* down/left */
	tx = x + 1;
	ty = y - 1;
	while (tx < 8 && ty >= 0 && a(board, tx, ty) == not_me) {
		tx++; ty--;
	}
	if (tx < 8 && ty >= 0 && ty != y - 1 && tx != x + 1 && 
			a(board, tx, ty) == me) {
		if (domove == false)
			return true;

		tx = x + 1;
		ty = y - 1;
		while (tx < 8 && ty >= 0 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx++; ty--;
		}
		flipped++;
	}

	if (domove == false) 
		return false;

	a(board, x, y) = me;
	if (flipped == 0) 
		return false;
	return true;
}
