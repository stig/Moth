/* 
 * moth -- my/mini Othello
 * Copyright (C) 2003 Stig Brautaset
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
#include "config-include/config-options.h"


/* prototype for callbacks */
int make_move(void *boarddata, const void *movedata, int me);
int end_of_game(const void *boarddata, int me);
void find_moves(struct ggtl *game, const void *boarddata, int me);
int evaluate(const void *boarddata, int me);

static int valid_move(char *board, int x, int y, int me, int domove);
void display(const void *boarddata);
void mainloop(struct ggtl *game, int ply1, int ply2);
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


void greeting(void)
{
	puts("This is moth, an othello game for console.");
	puts("Copyright (C) 2003 Stig Brautaset.");
	puts("This is free software; see the source for details.\n");
	printf("Report bugs to <%s>.\n\n", cfg__moth_bug_email);
}


int main(int argc, char **argv)
{
	struct ggtl *game;
	char board[8][8] = {{0}};
	int ply1 = 3, ply2 = 3;

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
	mainloop(game, ply1, ply2);

	return 0;
}


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
	

void mainloop(struct ggtl *game, int ply1, int ply2)
{	
	char move[128] = {0};
	const void *board;
	int score, maxply, player;

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
			maxply = ply1;
		}
		else {
			maxply = ply2;
		}

		printf("\nplayer %d (%c)\n", player, player==1?'-':'#');
		printf("Chose action (00-77|ENTER|undo|rate|save|load): ");
		fflush(stdout);
		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
                else if (!strncmp(move, "rate", 4)) {
                        printf("minimax value: %d\n\n", ggtl_rate_move(game, maxply));
                        board = NULL;
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
				board = ggtl_alphabeta(game, maxply);
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
	ggtl_free(game);
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
				printf(" %c |", c==1?'-':'#');
			else 
				printf("   |");
		}
		puts("\n  +---+---+---+---+---+---+---+---+");

	}
}


/* 
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


void find_moves(struct ggtl *game, const void *boarddata, int me)
{
	const char *board = boarddata;
	char mv[2];
	int i, j, cnt;
	
	cnt = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me, 0)) {
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


int end_of_game(const void *boarddata, int me)
{
	const char *board = boarddata;
	int i, j, not_me = 3 - me;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(board, i, j, me, 0))
				return 0;
			if (valid_move(board, i, j, not_me, 0))
				return 0;
		}
	}
	return 1;
}


int make_move(void *boarddata, const void *movedata, int me)
{
	char *board = boarddata;
	const char *move = movedata;
	int y = move[0];
	int x = move[1];

	/* null or pass move */
	if (x == -1 && y == -1) 
		return 1;

	if (x < 0 || x > 7 || y < 0 || y > 7) 
		return 0;

	return valid_move(board, x, y, me, 1);
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


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
static int valid_move(char *board, int x, int y, int me, int domove)
{
	int not_me = 3 - me;
	int tx, ty, flipped = 0;

	/* slot must not already be occupied */
	if (a(board, x, y) != 0)
		return 0;

	/* left */
	for (tx = x - 1; tx >= 0 && a(board, tx, y) == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && a(board, tx, y) == me) {
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

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
		if (domove == 0)
			return 1;

		tx = x + 1;
		ty = y - 1;
		while (tx < 8 && ty >= 0 && a(board, tx, ty) == not_me) {
			a(board, tx, ty) = me;
			tx++; ty--;
		}
		flipped++;
	}

	if (domove == 0) 
		return 0;

	a(board, x, y) = me;
	if (flipped == 0) 
		return 0;
	return 1;
}
