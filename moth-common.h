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

#ifdef __cplusplus	/* let C++ coders use this library */
extern "C" {
#endif


/*
 * Used to simulate multi-dimensional array for a one-dimensional one
 */
#define a(A, B, C) A[(B) * 8 + (C)]

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
int evaluate(const void *boarddata, int me);


/* 
 * Find and add possible moves at this position to GGTL's internal
 * lists.
 */
void find_moves(struct ggtl *game, const void *boarddata, int me);


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(const void *boarddata, int me);


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int make_move(void *boarddata, const void *movedata, int me);


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int valid_move(const char *board, int x, int y, int me);


/* 
 * Count the number of pieces on the board for the given player
 */
int count_pieces(const void *boarddata, int me);


/* 
 * Get a line of input
 */
int getline(char *s, int size);


/* end prototype section */

#ifdef __cplusplus
}
#endif
#endif /* GGTL__GGTL_H */
