/* 
 * Connect4 
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
#include <connect4/libc4.h>

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

	if (!*fixed) {
		*level1 *= 10;
		*level2 *= 10;
	}
}


static int mkscore(int exp);

int save(char *fn, struct ggtl *g)
{
	FILE *fp;
	int retval = 0;
	
	fp = fopen(fn, "w");
	if (!fp) 
		return -1;

	if (ggtl_save(g, fp, write_pos, write_move)) {
		puts("foo1");
		retval = -1;
	}

	if (fclose(fp)) {
		puts("foo2");
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
	if (fprintf(fp, "col: %d\n", mv->col) < 0)
		return -1;
	return 0;
}


struct ggtl_move *read_move(FILE *fp)
{
	struct ggtl_move *mv;
	int col;

	if (fscanf(fp, "col: %d\n", &col) != 1)
		return NULL;

	mv = malloc(sizeof *mv);
	if (!mv) 
		return NULL;

	mv->next = NULL;
	mv->col = col;

	return mv;
}


int write_pos(FILE *fp, struct ggtl_pos *pos)
{
	int i, j, player;

	if (!pos)
		return -1;
	if (fprintf(fp, "player start: %d\n", pos->player) < 0)
		return -1;

	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
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

	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
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


int findscore(struct ggtl_pos *b, int me)
{
	int xoff, yoff;
	int i, j, c, not_me = 3 - me;
	int total_score, score;

	total_score = 0;

	/* vertical */
	for (yoff = 0; yoff < 2; yoff++) {
		for (i = 0; i < COLS; i++) {
			score = 0;
			for (j = 0; j < 4; j++) {
				c = b->b[j + yoff][i];
				if (c == not_me) 
					score = -10;
				else if (c == me) 
					score++;
			}
			if (score > 0) {
				if (score == 4) 
					return GGTL_MAX;
				total_score += mkscore(score);
			}
		}
	}

	/* horizontal */
	for (xoff = 0; xoff < 4; xoff++) {
		for (i = 0; i < ROWS; i++) {
			score = 0;
			for (j = 0; j < 4; j++) {
				c = b->b[i][j + xoff];
				if (c == not_me) 
					score = -10;
				else if (c == me) 
					score++;
			}
			if (score > 0) {
				if (score == 4) 
					return GGTL_MAX;
				total_score += mkscore(score);
			}
		}
	}

	/* diagonals */
	for (yoff = 0; yoff < 2; yoff++) {
		for (xoff = 0; xoff < 4; xoff++) {

			/* diagonal 1 */
			score = 0;
			for (i = 0; i < 4; i++) {
				c = b->b[i + yoff][i + xoff];
				if (c == not_me) {
					score = -10;
				}
				else if (c == me) {
					score++;
				}
				if (score > 0) {
					if (score == 4) 
						return GGTL_MAX;
					total_score += mkscore(score);
				}
			}

			/* diagonal 2 */
			score = 0;
			for (i = 0; i < 4; i++) {
				c = b->b[3-i + yoff][i + xoff];
				if (c == not_me) {
					score = -10;
				}
				else if (c == me) {
					score++;
				}
				if (score > 0) {
					if (score == 4) 
						return GGTL_MAX;
					total_score += mkscore(score);
				}
			}
		}
	}
	return total_score;
}


/* 
 * Evaluate a board position for the given player.
 */
int evaluate(struct ggtl_pos *b)
{
	int myscore = findscore(b, b->player);
	int notmyscore = findscore(b, 3 - b->player);

	if (myscore == GGTL_MAX)
		return GGTL_MAX;
	if (notmyscore == GGTL_MAX)
		return GGTL_MIN;
	return myscore - notmyscore;
}


/* 
 * Return zero if the game has _not_ ended at this position (for the
 * current player), and non-zero if it has.
 */
int end_of_game(struct ggtl_pos *b)
{
	int i;

	if (GGTL_MAX == findscore(b, b->player))
		return GGTL_MAX;

	if (GGTL_MAX == findscore(b, 3 - b->player))
		return GGTL_MIN;

	for (i = 0; i < COLS; i++) {
		if (b->b[0][i] == 0) {
			return 0;
		}
	}
	return 1;
}


/* 
 * Find and add possible moves at this position to GGTL's internal
 * lists.
 */
void find_moves(struct ggtl *game, struct ggtl_pos *b)
{
	struct ggtl_move *mv;
	int i;
	
	for (i = 0; i < COLS; i++) {
		if (b->b[0][i] == 0) {
			mv = ggtl_pop_move(game);
			if (!mv)
				mv = ensure_move();
			mv->col = i;
			ggtl_add_move(game, mv);
		}
	}
}


/* 
 * Apply a move to a state 
 */
struct ggtl_pos *make_move(struct ggtl *g, struct ggtl_pos *b, struct ggtl_move *m)
{
	int i, mv = m->col;
	struct ggtl_pos *pos;

	pos = ggtl_pop_pos(g);
	pos = copy_pos(pos, b);

	if (mv < 0 || mv >= COLS) {
		ggtl_push_pos(g, pos);
		return NULL;
	}

	/* slot must not already be occupied */
	for (i = ROWS-1; pos->b[i][mv] != 0 && i >= 0; i--)
		;

	if (i >= 0 && !pos->b[i][mv]) {
		pos->b[i][mv] = pos->player;
		pos->player = 3 - pos->player;
		return pos;
	}
	ggtl_push_pos(g, pos);
	return NULL;
}


/* 
 * Print greeting and warranty details.
 */
void greeting(void)
{
	puts("This is connect4.");
	puts("Copyright (C) 2003 Stig Brautaset, Dimitris Parapadakis &");
	puts("the University of Westminster, London, UK.");
	puts("This is free software; see the source for details.\n");
	puts("Report bugs to <stig@brautaset.org>.\n");
}


void gameover(struct ggtl_pos *b)
{
        if (GGTL_MAX == findscore(b, 1)) {
                puts("Player 1 won!");
        }
        else if (GGTL_MAX == findscore(b, 2)) {
                puts("Player 2 won!");
        }
        else {
                puts("The game ended in a draw\n\n");
        }
}


/* 
 * Draw a game position on screen.
 */
void display(struct ggtl_pos *b)
{
	int i, j, c;

	puts("");
	for (i = 0; i < COLS; i++)
		printf("  %d ", i);
	puts("\n+---+---+---+---+---+---+---+");

	for (i = 0; i < ROWS; i++) {
		putchar('|');
		for (j = 0; j < COLS; j++) {
			c = b->b[i][j];
			if (c == 1)
				printf(" - |");
			else if (c == 2) 
				printf(" X |");
			else 
				printf("   |");
		}
		puts("\n+---+---+---+---+---+---+---+");

	}
}


static int mkscore(int exp)
{
	int score = 1;
	while (--exp > 0)
		score *= 10;
	return score;
}

/* arch-tag: Stig Brautaset Mon Apr 14 13:11:03 BST 2003
 */
