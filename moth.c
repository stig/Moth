#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ggtl.h>

/* prototype for callbacks */
bool end_of_game(void *boarddata, int me);
nodeptr find_moves(void *boarddata, nodeptr *availmoves, int me);
bool make_move(void *boarddata, void *movedata, int me);
int evaluate(void *boarddata, int me);

static bool valid_move(char *board, int x, int y, int me, bool domove);
void display(const void *boarddata);
void mainloop(struct ggtl *game, int maxply);
static int count_pieces(const void *boarddata, int me);

#define a(A, B, C) A[(B) * 8 + (C)]

/* One global variable -- whether this should remain global or not is
 * uncertain -- I need to profile the code to know that. Intuitively it
 * should be faster not having to create this on every entry of the
 * evaluation function, but that might not be the case anyway.
 */
const int heuristic[8][8] = {	{9, 2, 7, 8, 8, 7, 2, 9},
				{2, 1, 3, 4, 4, 3, 1, 2},
				{7, 3, 6, 5, 5, 6, 3, 7},
				{8, 4, 5, 1, 1, 5, 4, 8},
				{8, 4, 5, 1, 1, 5, 4, 8},
				{7, 3, 6, 5, 5, 6, 3, 7},
				{2, 1, 3, 4, 4, 3, 1, 2},
				{9, 2, 7, 8, 8, 7, 2, 9} 
};

int main(int argc, char **argv)
{
	struct ggtl *game;
	char board[8][8] = {{0}};
	int ply = 3;

	board[3][4] = board[4][3] = 1;
	board[3][3] = board[4][4] = 2;

	game = ggtl_new(board, sizeof board, 2);
	ggtl_add_callbacks(game, end_of_game, find_moves, make_move, evaluate);

	if (argc > 1) {
		ply = atoi(argv[1]);
	}
	mainloop(game, ply);

	return 0;
}

void mainloop(struct ggtl *game, int maxply)
{	
	char move[100];
	void *board;
	bool show;
	int tmp;

	board = ggtl_peek_current_state(game);
	do {
		show = true;
#if 0
		fputs("Chose action (00-77 / undo / eval): ", stdout);
		fflush(stdout);
		fgets(move, sizeof move, stdin);
#endif
		if (!strncmp(move, "undo", 4)) {
                        if (!ggtl_undo_move(game)) {
                                puts("Error: no move to undo\n");
                                show = false;
                        }
                }
                else if (!strncmp(move, "eval", 4)) {
                        printf("minimax value: %d\n\n", evaluate(board, 1));
                        show = false;
                }
                else if (ggtl_make_move(game, move)) {
                        show = true;
                }
                else if (ggtl_alphabeta(game, maxply)) {
                        show = true;
                }

                board = ggtl_peek_current_state(game);
                if (show == true) {
                        display(board);
                }
	} while (!end_of_game(board, 1));

	tmp = count_pieces(board, 1);
	tmp -= count_pieces(board, 2);

	if (tmp > 0) {
		printf("Player 1 won, with a margin of %d\n\n", tmp);
	}
	else if (tmp < 0) {
		printf("Player 2 won, with a margin of %d\n\n", -tmp);
	}
	else {
		puts("The game ended in a draw\n\n");
	}
}

void display(const void *boarddata)
{
	const char *board = boarddata;
	int i, j, c;

	printf("\n   ");
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

int evaluate(void *boarddata, int me)
{
	const char *board = boarddata;
	int not_me = 3 - me;
	int i, j, c, score = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = a(board, i, j);
			if (c == me) {
				score += heuristic[i][j];
				score++;
			}
			else if (c == not_me) {
				score -= heuristic[i][j];
				score++;
			}
		}
	}
	return score;
}


nodeptr find_moves(void *boarddata, nodeptr *availmoves, int me)
{
	nodeptr tmp, movelist = NULL;
	char *board = boarddata;
	char *mv, i, j;
	
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me, false)) {
				tmp = sll_pop(availmoves); 
				assert(tmp != NULL); 
				mv = sll_peek(tmp); 
				assert(mv != NULL);
				mv[0] = i + '0'; 
				mv[1] = j + '0'; 
				sll_push(&movelist, tmp); 
			}
		}
	}

	if (!movelist) {
		tmp = sll_pop(availmoves);
		mv = sll_peek(tmp);
		mv[0] = mv[1] = -1 + '0';
		sll_push(&movelist, tmp);
	}

	return movelist;
}

bool end_of_game(void *boarddata, int me)
{
	char *board = boarddata;
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

bool make_move(void *boarddata, void *movedata, int me)
{
	char *board = boarddata;
	const char *move = movedata;
	int x, y;

	x = move[0] - '0';
	y = move[1] - '0';

	if (x == -1 && y == -1) 
		return true;
	return valid_move(board, x, y, me, true);
}

static int count_pieces(const void *boarddata, int me)
{
	const char *board = boarddata;
	int i, j, count = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (a(board, i, j) == me)
				count++;
		}
	}
	return count;
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
	if (tx >= 0 && ty >= 0 && tx != x - 1 && ty != y - 1 && 
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
	if (tx >= 0 && ty < 8 && tx != x - 1 && ty != y + 1 && 
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
	if (tx < 8 && ty < 8 && tx != x + 1 && ty != y + 1 && 
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
	if (tx < 8 && ty >= 0 && tx != x + 1 && ty != y - 1 && 
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
