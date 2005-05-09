/* 
 * moth -- an Othello game for console
 * Copyright (C) 2003 Stig Brautaset, Dimitris Parapadakis and the
 * University of Westminster, London, UK.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ggtl/ggtl.h"
#include "config-options.h"


/* prototype for callbacks */
int make_move(void *boarddata, const void *movedata, int me);
int end_of_game(const void *boarddata, int me);
void find_moves(struct ggtl *game, const void *boarddata, int me);
int evaluate(const void *boarddata, int me);

static int count_pieces(const void *boarddata, int me);
static int valid_move(const char *board, int x, int y, int me);
static int getline(char *s, int size);

/*
 * Used to simulate multi-dimensional array for a one-dimensional one
 */
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


void greeting(void)
{
	puts("This is moth, an othello game for console.");
	puts("Copyright (C) 2003 Stig Brautaset.");
	puts("This is free software; see the source for copying conditions.");
	puts("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A");
	puts("PARTICULAR PURPOSE.\n");
	printf("Please report bugs to <%s>.\n\n", cfg__moth_bug_email);
}


/* 
 * Draw a game position on screen.
 */
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
				printf(" %c |", c==1?'-':'#');
			else 
				printf("   |");
		}
		puts("\n  +---+---+---+---+---+---+---+---+");

	}
}


/* 
 * This function actually plays the game.
 */
struct ggtl *mainloop(struct ggtl *game, int ply1, int ply2)
{	
	char move[128] = {0};
	const void *board;
	int score, player;

	board = ggtl_peek_state(game);
	for (;;) {
		if (board) {
			display(board);
		}

		board = ggtl_peek_state(game);
		if (end_of_game(board, 1) && end_of_game(board, 2)) {
			break;
		}

		player = ggtl_get(game, GGTL_PLAYER_TURN); 
		if (player == 1) {
			ggtl_set(game, GGTL_PLY_TIMELIM, ply1);
		}
		else {
			ggtl_set(game, GGTL_PLY_TIMELIM, ply2);
		}

		printf("\nplayer %d (%c)\n", player, player==1?'-':'#');
		printf("Chose action (00-77|ENTER|undo|rate|redisp|save|load): ");
		fflush(stdout);
		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
                else if (!strncmp(move, "rate", 4)) {
                        printf("minimax value: %d\n\n", ggtl_rate_move(game));
                        board = NULL;
                }
                else if (!strncmp(move, "redisp", 6)) {
                        board = ggtl_peek_state(game);
                }
		else if (!strncmp(move, "save", 4)) {
			printf("Saving game, need a name: "); fflush(stdout);
			getline(move, sizeof move);
			if (ggtl_save(game, move))
				puts("success");
			else puts("failed");
			board = NULL;
		}
		else if (!strncmp(move, "load", 4)) {
			struct ggtl *tmp;
			printf("Loading game, need a name: "); fflush(stdout);
			getline(move, sizeof move);
			tmp = ggtl_new(make_move, end_of_game, find_moves, evaluate);
			if (move[0] && tmp && (board = ggtl_resume(tmp, move))) {
				printf("loaded game from `%s'.", move);
				ggtl_free(game);
				game = tmp;
			}
			else {
				printf("failed loading game from `%s'.", move);
				ggtl_free(tmp);
				board = NULL;
			}
		}
                else {
			move[0] -= '0';
			move[1] -= '0';
			board = ggtl_move(game, move);
			
			if (!board) {
				board = ggtl_alphabeta_iterative(game);
			}
		}
	} 

	score = count_pieces(board, 1);
	score -= count_pieces(board, 2);

	if (score > 0) {
		printf("Player 1 won, with a margin of %d\n\n", score);
	}
	else if (score < 0) {
		printf("Player 2 won, with a margin of %d\n\n", -score);
	}
	else {
		puts("The game ended in a draw\n\n");
	}
	return game;
}


int main(int argc, char **argv)
{
	struct ggtl *game;
	char board[8][8] = {{0}};
	int ply1 = 30, ply2 = 30;

	board[3][4] = board[4][3] = 1;
	board[3][3] = board[4][4] = 2;

	greeting();

	game = ggtl_new(make_move, end_of_game, find_moves, evaluate);
	if (!ggtl_init(game, board, sizeof board, 2)) {
		ggtl_free(game);
		puts("sorry -- NO GAME FOR YOU!");
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		ply1 = atoi(argv[1]);
	}
	if (argc > 2) {
		ply2 = atoi(argv[2]);
	}

	srand(time(NULL));
	game = mainloop(game, ply1, ply2);
	ggtl_free(game);

	return 0;
}


/* 
 * Evaluate a board position for the given player.
 * 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int evaluate(const void *boarddata, int me)
{
	const char *board = boarddata;
	int c, i, j;
	int not_me = 3 - me;
	int myscore = 0, notmyscore = 0;

#if 1
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = a(board, i, j);
			if (c == me) {
				myscore += heuristic[i][j];
			}
			else if (c == not_me) {
				notmyscore += heuristic[i][j];
			}
		}
	}
#else
	myscore = count_pieces(board, me);
	notmyscore = count_pieces(board, not_me);
#endif
	if (!myscore) return GGTL_MIN;
	if (!notmyscore) return GGTL_MAX;
	return myscore - notmyscore;
}


/* 
 * Find and add possible moves at this position to GGTL's internal
 * lists.
 */
void find_moves(struct ggtl *game, const void *boarddata, int me)
{
	const char *board = boarddata;
	char mv[2];
	int i, j, cnt;
	
	cnt = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me)) {
				mv[0] = (char)j; 
				mv[1] = (char)i; 
				ggtl_add_move(game, mv);
				cnt++;
			}
		}
	}

	if (!cnt) {
		mv[0] = mv[1] = -1;
		ggtl_add_move(game, mv);
	}
}


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(const void *boarddata, int me)
{
	const char *board = boarddata;
	int i, j, not_me = 3 - me;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me))
				return 0;
			if (valid_move(board, i, j, not_me))
				return 0;
		}
	}
	return 1;
}


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int make_move(void *boarddata, const void *movedata, int me)
{
	char *board = boarddata;
	const char *move = movedata;
	int tx, ty, flipped = 0;
	int not_me = 3 - me;
	int y = move[0];
	int x = move[1];

	/* null or pass move */
	if (x == -1 && y == -1) 
		return 1;

	if (x < 0 || x > 7 || y < 0 || y > 7) 
		return 0;

	/* slot must not already be occupied */
	if (a(board, x, y) != 0)
		return 0;

	/* left */
	for (tx = x - 1; tx >= 0 && a(board, tx, y) == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && a(board, tx, y) == me) {
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
		tx = x + 1;
		ty = y - 1;
		while (tx < 8 && ty >= 0 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx++; ty--;
		}
		flipped++;
	}

	if (flipped == 0) 
		return 0;

	a(board, x, y) = me;
	return 1;
}


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
static int valid_move(const char *board, int x, int y, int me)
{
	int tx, ty;
	int not_me = 3 - me;

	/* slot must not already be occupied */
	if (a(board, x, y) != 0)
		return 0;

	/* left */
	for (tx = x - 1; tx >= 0 && a(board, tx, y) == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && a(board, tx, y) == me) 
		return 1;

	/* right */
	for (tx = x + 1; tx < 8 && a(board, tx, y) == not_me; tx++)
		;
	if (tx < 8 && tx != x + 1 && a(board, tx, y) == me)
		return 1;

	/* up */
	for (ty = y - 1; ty >= 0 && a(board, x, ty) == not_me; ty--)
		;
	if (ty >= 0 && ty != y - 1 && a(board, x, ty) == me) 
		return 1;
	
	/* down */
	for (ty = y + 1; ty < 8 && a(board, x, ty) == not_me; ty++)
		;
	if (ty < 8 && ty != y + 1 && a(board, x, ty) == me) 
		return 1;
	
	/* up/left */
	tx = x - 1;
	ty = y - 1; 
	while (tx >= 0 && ty >= 0 && a(board, tx, ty) == not_me) {
		tx--; ty--;
	}
	if (tx >= 0 && ty >= 0 && tx != x - 1 && ty != y - 1 && 
			a(board, tx, ty) == me)
		return 1;

	/* up/right */
	tx = x - 1;
	ty = y + 1; 
	while (tx >= 0 && ty < 8 && a(board, tx, ty) == not_me) {
		tx--; ty++;
	}
	if (tx >= 0 && ty < 8 && tx != x - 1 && ty != y + 1 && 
			a(board, tx, ty) == me)
		return 1;
	
	/* down/right */
	tx = x + 1;
	ty = y + 1; 
	while (tx < 8 && ty < 8 && a(board, tx, ty) == not_me) {
		tx++; ty++;
	}
	if (tx < 8 && ty < 8 && tx != x + 1 && ty != y + 1 && 
			a(board, tx, ty) == me) 
		return 1;

	/* down/left */
	tx = x + 1;
	ty = y - 1;
	while (tx < 8 && ty >= 0 && a(board, tx, ty) == not_me) {
		tx++; ty--;
	}
	if (tx < 8 && ty >= 0 && tx != x + 1 && ty != y - 1 && 
			a(board, tx, ty) == me)
		return 1;

	/* if we get here the move was illegal */
	return 0;
}


/* 
 * Count the number of pieces on the board for the given player
 */
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


/* 
 * Get a line of input
 */
int getline(char *s, int size)
{
	char fmt[50];
	int ret;
	snprintf(fmt, sizeof fmt, "%%%d[^\n]%%*[^\n]", size);
	*s = '\0';
	ret = scanf(fmt, s);
	getchar();
	return ret;
}

