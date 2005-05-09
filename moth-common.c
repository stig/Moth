/* 
 * moth -- an Othello game 
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

#include <stdio.h>
#include "moth/moth-common.h"
#include "config-options.h"


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


/* 
 * Print greeting and warranty details.
 */
void greeting(void)
{
	puts("This is moth, yet another othello game.");
	puts("Copyright (C) 2003 Stig Brautaset, Dimitris Parapadakis &");
	puts("the University of Westminster, London, UK.");
	puts("This is free software; see the source for details.\n");
	printf("Report bugs to <%s>.\n\n", cfg__moth_bug_email);
}


/* 
 * Evaluate a board position for the given player.
 * 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int evaluate(const struct ggtl_state *b)
{
	int c, i, j;
	int me = b->player;
	int not_me = 3 - me;
	int myscore = 0, notmyscore = 0;

#if 1
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = b->b[i][j];
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
void find_moves(struct ggtl *game, const struct ggtl_state *b)
{
	struct ggtl_move mv;
	int me = b->player;
	int i, j, cnt;
	
	cnt = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(b, i, j, me)) {
				mv.x = i; 
				mv.y = j; 
				ggtl_add_move(game, &mv);
				cnt++;
			}
		}
	}

	if (!cnt) {
		/* add a pass move */
		mv.x = mv.y = -1;
		ggtl_add_move(game, &mv);
	}
}


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(const struct ggtl_state *b)
{
	int i, j;
	int me = b->player;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(b, i, j, me))
				return 0;
		}
	}
	return 1;
}


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int make_move(struct ggtl_state *b, const struct ggtl_move *m)
{
	int me = b->player;
	int not_me = 3 - me;
	int tx, ty, flipped = 0;
	int x = m->x;
	int y = m->y;

	/* null or pass move */
	if (x == -1 && y == -1) {
		b->player = 3 - me;
		return 1;
	}

	if (x < 0 || x > 7 || y < 0 || y > 7) 
		return 0;

	/* slot must not already be occupied */
	if (b->b[x][y] != 0)
		return 0;

	/* left */
	for (tx = x - 1; tx >= 0 && b->b[tx][y] == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && b->b[tx][y] == me) {
		tx = x - 1;
		while (tx >= 0 && b->b[tx][y] == not_me) {
			b->b[tx][y] = me;
			tx--;
		}
		flipped++;
	}

	/* right */
	for (tx = x + 1; tx < 8 && b->b[tx][y] == not_me; tx++)
		;
	if (tx < 8 && tx != x + 1 && b->b[tx][y] == me) {
		tx = x + 1;
		while (tx < 8 && b->b[tx][y] == not_me) {
			b->b[tx][y] = me;
			tx++;
		}
		flipped++;
	}

	/* up */
	for (ty = y - 1; ty >= 0 && b->b[x][ty] == not_me; ty--)
		;
	if (ty >= 0 && ty != y - 1 && b->b[x][ty] == me) {
		ty = y - 1;
		while (ty >= 0 && b->b[x][ty] == not_me) {
			b->b[x][ty] = me;
			ty--;
		}
		flipped++;
	}
	
	/* down */
	for (ty = y + 1; ty < 8 && b->b[x][ty] == not_me; ty++)
		;
	if (ty < 8 && ty != y + 1 && b->b[x][ty] == me) {
		ty = y + 1;
		while (ty < 8 && b->b[x][ty] == not_me) {
			b->b[x][ty] = me;
			ty++;
		}
		flipped++;
	}
	
	/* up/left */
	tx = x - 1;
	ty = y - 1; 
	while (tx >= 0 && ty >= 0 && b->b[tx][ty] == not_me) {
		tx--; ty--;
	}
	if (tx >= 0 && ty >= 0 && tx != x - 1 && ty != y - 1 && 
			b->b[tx][ty] == me) {
		tx = x - 1;
		ty = y - 1;
		while (tx >= 0 && ty >= 0 && b->b[tx][ty] == not_me) {
			b->b[tx][ty] = me;
			tx--; ty--;
		}
		flipped++;
	}

	/* up/right */
	tx = x - 1;
	ty = y + 1; 
	while (tx >= 0 && ty < 8 && b->b[tx][ty] == not_me) {
		tx--; ty++;
	}
	if (tx >= 0 && ty < 8 && tx != x - 1 && ty != y + 1 && 
			b->b[tx][ty] == me) {
		tx = x - 1;
		ty = y + 1;
		while (tx >= 0 && ty < 8 && b->b[tx][ty] == not_me) {
			b->b[tx][ty] = me;
			tx--; ty++;
		}
		flipped++;
	}
	
	/* down/right */
	tx = x + 1;
	ty = y + 1; 
	while (tx < 8 && ty < 8 && b->b[tx][ty] == not_me) {
		tx++; ty++;
	}
	if (tx < 8 && ty < 8 && tx != x + 1 && ty != y + 1 && 
			b->b[tx][ty] == me) {
		tx = x + 1;
		ty = y + 1;
		while (tx < 8 && ty < 8 && b->b[tx][ty] == not_me) {
			b->b[tx][ty] = me;
			tx++; ty++;
		}
		flipped++;
	}

	/* down/left */
	tx = x + 1;
	ty = y - 1;
	while (tx < 8 && ty >= 0 && b->b[tx][ty] == not_me) {
		tx++; ty--;
	}
	if (tx < 8 && ty >= 0 && tx != x + 1 && ty != y - 1 && 
			b->b[tx][ty] == me) {
		tx = x + 1;
		ty = y - 1;
		while (tx < 8 && ty >= 0 && b->b[tx][ty] == not_me) {
			b->b[tx][ty] = me;
			tx++; ty--;
		}
		flipped++;
	}

	if (flipped == 0) 
		return 0;

	b->b[x][y] = me;
	b->player = 3 - me;
	return 1;
}


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int valid_move(const struct ggtl_state *b, int x, int y, int me)
{
	int tx, ty;
	int not_me = 3 - me;

	/* slot must not already be occupied */
	if (b->b[x][y] != 0)
		return 0;

	/* left */
	for (tx = x - 1; tx >= 0 && b->b[tx][y] == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && b->b[tx][y] == me) 
		return 1;

	/* right */
	for (tx = x + 1; tx < 8 && b->b[tx][y] == not_me; tx++)
		;
	if (tx < 8 && tx != x + 1 && b->b[tx][y] == me)
		return 1;

	/* up */
	for (ty = y - 1; ty >= 0 && b->b[x][ty] == not_me; ty--)
		;
	if (ty >= 0 && ty != y - 1 && b->b[x][ty] == me) 
		return 1;
	
	/* down */
	for (ty = y + 1; ty < 8 && b->b[x][ty] == not_me; ty++)
		;
	if (ty < 8 && ty != y + 1 && b->b[x][ty] == me) 
		return 1;
	
	/* up/left */
	tx = x - 1;
	ty = y - 1; 
	while (tx >= 0 && ty >= 0 && b->b[tx][ty] == not_me) {
		tx--; ty--;
	}
	if (tx >= 0 && ty >= 0 && tx != x - 1 && ty != y - 1 && 
			b->b[tx][ty] == me)
		return 1;

	/* up/right */
	tx = x - 1;
	ty = y + 1; 
	while (tx >= 0 && ty < 8 && b->b[tx][ty] == not_me) {
		tx--; ty++;
	}
	if (tx >= 0 && ty < 8 && tx != x - 1 && ty != y + 1 && 
			b->b[tx][ty] == me)
		return 1;
	
	/* down/right */
	tx = x + 1;
	ty = y + 1; 
	while (tx < 8 && ty < 8 && b->b[tx][ty] == not_me) {
		tx++; ty++;
	}
	if (tx < 8 && ty < 8 && tx != x + 1 && ty != y + 1 && 
			b->b[tx][ty] == me) 
		return 1;

	/* down/left */
	tx = x + 1;
	ty = y - 1;
	while (tx < 8 && ty >= 0 && b->b[tx][ty] == not_me) {
		tx++; ty--;
	}
	if (tx < 8 && ty >= 0 && tx != x + 1 && ty != y - 1 && 
			b->b[tx][ty] == me)
		return 1;

	/* if we get here the move was illegal */
	return 0;
}


/* 
 * Count the number of pieces on the board for the given player
 */
int count_pieces(const struct ggtl_state *b, int me)
{
	int i, j, count = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (b->b[i][j] == me)
				count++;
		}
	}
	return count;
}

