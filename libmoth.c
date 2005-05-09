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
#include <stdlib.h>
#include <options/opt.h>
#include <moth/libmoth.h>

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



void getopts(int argc, char **argv, int *debug, int *fixed, int *level1, int *level2)
{
	struct opt *opts;
	int help, longhelp, error;
	struct opt_defs options[] = {
		{"help", "h", 0, "0",	"Print a short help message and exit"},
		{"longhelp", "H", 0, "0","Print help with default values and exit"},
		{"debug", "d", 0, "0",	"Print debug level messages"},
		{"fixed", "f", 0, "0",	"Fixed-depth search (turn off iterative deepening)"},
		{"level1", "1", 1, "3",	"Depth of search (times 10ms for iterative deepening) -- player 1"},
		{"level2", "2", 1, "3",	"Depth of search (times 10ms for iterative deepening) -- player 2"},
		OPT_DEFS_END
	};

	opts = opt_init(options);
	if (!opts) {
		fputs("Option parsing initialisation failure\n", stderr);
		exit(EXIT_FAILURE);
	}

	if ((error = opt_parse(opts, &argc, argv, 0))) {
		fprintf(stderr, "Failure parsing options: %s\n", 
				opt_strerror(error));
		exit(EXIT_FAILURE);
	}

	error |= opt_val(opts, "help", "int", &help);
	error |= opt_val(opts, "longhelp", "int", &longhelp);
	error |= opt_val(opts, "debug", "int", debug);
	error |= opt_val(opts, "fixed", "int", fixed);
	error |= opt_val(opts, "level1", "str", level1);
	error |= opt_val(opts, "level2", "str", level2);
	if (error) {
		fprintf(stderr, "Failure retrieving values. ");
		fprintf(stderr, "Last error was: %s\n", opt_strerror(error));
		exit(EXIT_FAILURE);
	}
	opt_free(opts);

	if (help || longhelp) {
		opt_desc(options, longhelp);
		exit(EXIT_SUCCESS);
	}

	if (!fixed) {
		*level1 *= 10;
		*level2 *= 10;
	}
}

/* 
 * Print greeting and warranty details.
 */
void greeting(void)
{
	puts("This is moth, yet another othello game.");
	puts("Copyright (C) 2003 Stig Brautaset, Dimitris Parapadakis &");
	puts("the University of Westminster, London, UK.");
	puts("This is free software; see the source for details.\n");
	puts("Report bugs to <stig@brautaset.org>.\n");
}


int save(char *fn, struct ggtl *g)
{
	FILE *fp;
	int retval = 0;
	
	fp = fopen(fn, "w");
	if (!fp) 
		return -1;

	if (ggtl_save(g, fp, write_pos, write_move)) {
		retval = -1;
	}

	if (fclose(fp)) {
		retval = -1;
	}

	return retval;
}


struct ggtl *resume(char *fn)
{
	FILE *fp;
	struct ggtl *g;

	fp = fopen(fn, "r");
	if (!fp) 
		return NULL;

	g = ggtl_new(make_move, end_of_game, find_moves, evaluate);
	if (!g) {
		fclose(fp);
		return NULL;
	}

	if (!ggtl_resume(g, fp, read_pos, read_move)) {
		ggtl_free(g);
		fclose(fp);
		return NULL;
	}

	if (fclose(fp)) {
		ggtl_free(g);
		return NULL;
	}

	return g;
}


int write_move(FILE *fp, struct ggtl_move *mv)
{
	if (fprintf(fp, "x: %d, y: %d\n", mv->x, mv->y) < 0)
		return -1;
	return 0;
}


struct ggtl_move *read_move(FILE *fp)
{
	struct ggtl_move *mv;
	int x, y;

	if (fscanf(fp, "x: %d, y: %d\n", &x, &y) != 2)
		return NULL;

	mv = malloc(sizeof *mv);
	if (!mv) 
		return NULL;

	mv->next = NULL;
	mv->x = x;
	mv->y = y;

	return mv;
}


int write_pos(FILE *fp, struct ggtl_pos *pos)
{
	int i, j, player;

	if (!pos)
		return -1;
	if (fprintf(fp, "player start: %d\n", pos->player) < 0)
		return -1;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			player = pos->b[i][j];
			if (fputc('0' + player, fp) == EOF)
				return -1;

		}
		if (fputc('\n', fp) == EOF)
			return -1;
	}
	if (fputc('\n', fp) == EOF)
		return -1;
	return 0;
}


struct ggtl_pos *read_pos(FILE *fp)
{
	int i, j, player;
	struct ggtl_pos *pos; 
	
	if (fscanf(fp, "player start: %d\n", &player) != 1)
		return NULL;

	pos = malloc(sizeof *pos);
	if (!pos) 
		return NULL;
	pos->next = NULL;
	pos->player = player;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			player = fgetc(fp);
			if (player == EOF) {
				free(pos);
				return NULL;
			}

			pos->b[i][j] = player - '0';
		}
		if (fgetc(fp) != '\n') {
			free(pos);
			return NULL;
		}
	}
	if (fgetc(fp) != '\n') {
		free(pos);
		return NULL;
	}

	return pos;
}


/**
 * Copy a position. Use cached position if provided, else allocate the
 * necessary memory. Exit if the necessary memory cannot be allocated.
 *
 * @param dst where to copy the position (if non-NULL)
 * @param src the position to copy
 *
 * @return A pointer to the copy of the position is returned.
 */
struct ggtl_pos *copy_pos(struct ggtl_pos *dst, struct ggtl_pos *src)
{
	if (!dst) {
		dst = malloc(sizeof *dst);
		if (!dst)
			exit(EXIT_FAILURE);
		dst->next = NULL;
	}
	return memcpy(dst, src, sizeof *src);
}


struct ggtl_move *ensure_move(void)
{
	struct ggtl_move *pos;
 
	pos = malloc(sizeof *pos);
	if (!pos) 
		exit(EXIT_FAILURE);

	pos->next = NULL;
	return pos;
}


/* 
 * Evaluate a board position for the given player.
 * 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int evaluate(struct ggtl_pos *b)
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
void find_moves(struct ggtl *game, struct ggtl_pos *b)
{
	struct ggtl_move *mv;
	int me = b->player;
	int i, j, cnt;
	
	cnt = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (valid_move(b, i, j, me)) {
				mv = ggtl_pop_move(game);
				if (!mv) 
					mv = ensure_move();

				mv->x = i; 
				mv->y = j; 
				ggtl_add_move(game, mv);
				cnt++;
			}
		}
	}

	if (!cnt) {
		mv = ggtl_pop_move(game);
		if (!mv) 
			mv = ensure_move();

		/* add a pass move */
		mv->x = mv->y = -1;
		ggtl_add_move(game, mv);
	}
}


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(struct ggtl_pos *b)
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
struct ggtl_pos *make_move(struct ggtl *g, struct ggtl_pos *b, struct ggtl_move *m)
{
	int me = b->player;
	int not_me = 3 - me;
	int tx, ty, flipped = 0;
	int x = m->x;
	int y = m->y;
	struct ggtl_pos *pos;

	pos = ggtl_pop_pos(g);
	pos = copy_pos(pos, b);

	/* null or pass move */
	if (x == -1 && y == -1) {
		pos->player = 3 - me;
		return pos;
	}

	if (x < 0 || x > 7 || y < 0 || y > 7) {
		ggtl_push_pos(g, pos);
		return NULL;
	}

	/* slot must not already be occupied */
	if (pos->b[x][y] != 0) {
		ggtl_push_pos(g, pos);
		return NULL;
	}

	/* left */
	for (tx = x - 1; tx >= 0 && pos->b[tx][y] == not_me; tx--)
		;
	if (tx >= 0 && tx != x - 1 && pos->b[tx][y] == me) {
		tx = x - 1;
		while (tx >= 0 && pos->b[tx][y] == not_me) {
			pos->b[tx][y] = me;
			tx--;
		}
		flipped++;
	}

	/* right */
	for (tx = x + 1; tx < 8 && pos->b[tx][y] == not_me; tx++)
		;
	if (tx < 8 && tx != x + 1 && pos->b[tx][y] == me) {
		tx = x + 1;
		while (tx < 8 && pos->b[tx][y] == not_me) {
			pos->b[tx][y] = me;
			tx++;
		}
		flipped++;
	}

	/* up */
	for (ty = y - 1; ty >= 0 && pos->b[x][ty] == not_me; ty--)
		;
	if (ty >= 0 && ty != y - 1 && pos->b[x][ty] == me) {
		ty = y - 1;
		while (ty >= 0 && pos->b[x][ty] == not_me) {
			pos->b[x][ty] = me;
			ty--;
		}
		flipped++;
	}
	
	/* down */
	for (ty = y + 1; ty < 8 && pos->b[x][ty] == not_me; ty++)
		;
	if (ty < 8 && ty != y + 1 && pos->b[x][ty] == me) {
		ty = y + 1;
		while (ty < 8 && pos->b[x][ty] == not_me) {
			pos->b[x][ty] = me;
			ty++;
		}
		flipped++;
	}
	
	/* up/left */
	tx = x - 1;
	ty = y - 1; 
	while (tx >= 0 && ty >= 0 && pos->b[tx][ty] == not_me) {
		tx--; ty--;
	}
	if (tx >= 0 && ty >= 0 && tx != x - 1 && ty != y - 1 && 
			pos->b[tx][ty] == me) {
		tx = x - 1;
		ty = y - 1;
		while (tx >= 0 && ty >= 0 && pos->b[tx][ty] == not_me) {
			pos->b[tx][ty] = me;
			tx--; ty--;
		}
		flipped++;
	}

	/* up/right */
	tx = x - 1;
	ty = y + 1; 
	while (tx >= 0 && ty < 8 && pos->b[tx][ty] == not_me) {
		tx--; ty++;
	}
	if (tx >= 0 && ty < 8 && tx != x - 1 && ty != y + 1 && 
			pos->b[tx][ty] == me) {
		tx = x - 1;
		ty = y + 1;
		while (tx >= 0 && ty < 8 && pos->b[tx][ty] == not_me) {
			pos->b[tx][ty] = me;
			tx--; ty++;
		}
		flipped++;
	}
	
	/* down/right */
	tx = x + 1;
	ty = y + 1; 
	while (tx < 8 && ty < 8 && pos->b[tx][ty] == not_me) {
		tx++; ty++;
	}
	if (tx < 8 && ty < 8 && tx != x + 1 && ty != y + 1 && 
			pos->b[tx][ty] == me) {
		tx = x + 1;
		ty = y + 1;
		while (tx < 8 && ty < 8 && pos->b[tx][ty] == not_me) {
			pos->b[tx][ty] = me;
			tx++; ty++;
		}
		flipped++;
	}

	/* down/left */
	tx = x + 1;
	ty = y - 1;
	while (tx < 8 && ty >= 0 && pos->b[tx][ty] == not_me) {
		tx++; ty--;
	}
	if (tx < 8 && ty >= 0 && tx != x + 1 && ty != y - 1 && 
			pos->b[tx][ty] == me) {
		tx = x + 1;
		ty = y - 1;
		while (tx < 8 && ty >= 0 && pos->b[tx][ty] == not_me) {
			pos->b[tx][ty] = me;
			tx++; ty--;
		}
		flipped++;
	}

	if (flipped == 0) {
		ggtl_push_pos(g, pos);
		return NULL;
	}

	pos->b[x][y] = me;
	pos->player = 3 - me;

	return pos;
}


/* 
 * This function is heavily inspired by code in GNOME Iagno, which is
 * Copyright (C) Ian Peters <ipeters@acm.org> 
 */
int valid_move(struct ggtl_pos *b, int x, int y, int me)
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
int count_pieces(struct ggtl_pos *b, int me)
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

