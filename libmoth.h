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

#include <ggtl/ggtl.h>

struct ggtl_pos {
	struct ggtl_pos *next;
	int b[8][8];
	int player;
};

struct ggtl_move {
	struct ggtl_move *next;
	int x, y;
};

/* libmoth.c */
void greeting(void);
int save(char *fn, struct ggtl *g);
struct ggtl *resume(char *fn);
int write_move(FILE *fp, struct ggtl_move *mv);
struct ggtl_move *read_move(FILE *fp);
int write_pos(FILE *fp, struct ggtl_pos *pos);
struct ggtl_pos *read_pos(FILE *fp);
struct ggtl_pos *copy_pos(struct ggtl_pos *dst, struct ggtl_pos *src);
struct ggtl_move *ensure_move(void);
int evaluate(struct ggtl_pos *b);
void find_moves(struct ggtl *game, struct ggtl_pos *b);
int end_of_game(struct ggtl_pos *b);
struct ggtl_pos *make_move(struct ggtl *g, struct ggtl_pos *b, struct ggtl_move *m);
int valid_move(struct ggtl_pos *b, int x, int y, int me);
int count_pieces(struct ggtl_pos *b, int me);

#endif
