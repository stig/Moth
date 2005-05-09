/* 
 * Connect4 -- the classic game
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
#include <string.h>
#include <stdlib.h>
#include <connect4/libc4.h>



/* 
 * Get a line of input
 */
static int getline(char *s, size_t size)
{
	char fmt[50];
	int ret;
	sprintf(fmt, "%%%lu[^\n]%%*[^\n]", size);
	*s = '\0';
	ret = scanf(fmt, s);
	(void)getchar();
	return ret;
}


/* 
 * This function actually plays the game.
 */
static struct ggtl *mainloop(struct ggtl *game, int fixed, int ply1, int ply2)
{	
	char move[128] = {0};
	struct ggtl_pos *board;
	int ply;

	board = ggtl_peek_pos(game);
	for (;;) {
		if (board) {
			display(board);
		}

		board = ggtl_peek_pos(game);
		if (end_of_game(board)) {
			break;
		}
		printf("\nplayer %d (%c)\n", board->player, board->player==1?'-':'X');
		puts("Action (0-6|ENTER|undo|rate|redisp|save|load)?");

		if (board) 
			ply = board->player == 1 ? ply1 : ply2;

		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
                else if (!strncmp(move, "rate", 4)) {
                        printf("minimax value: %d\n\n", ggtl_rate_move(game, ply));
                        board = NULL;
                }
                else if (!strncmp(move, "redisp", 6)) {
                        board = ggtl_peek_pos(game);
                }
		else if (!strncmp(move, "save", 4)) {
			printf("Saving game, need a name: "); fflush(stdout);
			getline(move, sizeof move);
			if (!save(move, game))
				puts("success");
			else puts("failed");
			board = NULL;
		}
		else if (!strncmp(move, "load", 4)) {
			struct ggtl *tmp;
			printf("Loading game, need a name: "); fflush(stdout);
			getline(move, sizeof move);
			tmp = ggtl_new(make_move, end_of_game, find_moves, evaluate);
			if (move[0] && (tmp = resume(move))) {
				printf("loaded game from `%s'.", move);
				ggtl_free(game);
				game = tmp;
				board = ggtl_peek_pos(game);
			}
			else {
				printf("failed loading game from `%s'.", move);
				ggtl_free(tmp);
				board = NULL;
			}
		}
                else {
			struct ggtl_move *mv = ensure_move();
			mv->col = move[0] - '0';
			board = ggtl_move(game, mv);
			
			if (!board) {
				ggtl_push_move(game, mv);
				if (fixed) 
					board = ggtl_alphabeta(game, ply);
				else
					board = ggtl_alphabeta_iterative(game, ply);
				if (board) {
					printf("searched to ply %d\n",
						ggtl_get(game, GGTL_PLY_LAST));
				}
			}
		}
	} 

	gameover(board);
	return game;
}


int main(int argc, char **argv)
{
	struct ggtl *game;
	struct ggtl_pos *pos, start = {NULL, {{0}}, 1};
	int debug, fixed, level1, level2;

	greeting();
	getopts(argc, argv, &debug, &fixed, &level1, &level2);

	pos = copy_pos(NULL, &start);
	game = ggtl_new(make_move, end_of_game, find_moves, evaluate);
	if (!ggtl_init(game, pos)) {
		ggtl_free(game);
		puts("sorry -- NO GAME FOR YOU!");
		return EXIT_FAILURE;
	}
	ggtl_set(game, GGTL_DEBUG, debug);

	game = mainloop(game, fixed, level1, level2);
	ggtl_free(game);

	return 0;
}


/* arch-tag: Stig Brautaset Mon Apr 14 13:11:04 BST 2003
 */
