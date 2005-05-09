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

#ifndef MOTH__MOTH_COMMON_H
#define MOTH__MOTH_COMMON_H 1

#include "ggtl/ggtl.h"

struct ggtl_state {
	int b[8][8];
	int player;
};

struct ggtl_move {
	int x, y;
};

/* begin prototype section */
/* 
 * Print greeting and warranty details.
 */
void greeting(void);


/* 
 * Evaluate a board position for the given player.
 * 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int evaluate(const struct ggtl_state *b);


/* 
 * Find and add possible moves at this position to GGTL's internal
 * lists.
 */
void find_moves(struct ggtl *game, const struct ggtl_state *b);


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(const struct ggtl_state *b);


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int make_move(struct ggtl_state *b, const struct ggtl_move *m);


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int valid_move(const struct ggtl_state *b, int x, int y, int me);


/* 
 * Count the number of pieces on the board for the given player
 */
int count_pieces(const struct ggtl_state *b, int me);


/* end prototype section */

#endif
