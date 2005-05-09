/* 
 * moth -- an Othello game (console version)
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
#include "moth/moth-common.h"
#include "config-include/config-options.h"



/* 
 * Draw a game position on screen.
 */
static void display(const struct ggtl_pos *b)
{
	int i, j, c;

	printf("\n   ");
	for (i = 0; i < 8; i++)
		printf(" %d  ", i);
	puts("\n  +---+---+---+---+---+---+---+---+");

	for (i = 0; i < 8; i++) {
		printf("%d |", i);
		for (j = 0; j < 8; j++) {
			c = b->b[i][j];
			if (c == 1)
				printf(" - |");
			else if (c == 2) 
				printf(" X |");
			else 
				printf("   |");
		}
		puts("\n  +---+---+---+---+---+---+---+---+");

	}
}


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
	getchar();
	return ret;
}


/* 
 * This function actually plays the game.
 */
static struct ggtl *mainloop(struct ggtl *game, int ply1, int ply2)
{	
	char move[128] = {0};
	const struct ggtl_pos *board;
	int score;

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
		puts("Action (00-77|ENTER|undo|rate|redisp|save|load)?");


		if (board) {
			if (board->player == 1) {
				ggtl_set(game, GGTL_PLY_TIMELIM, ply1);
				ggtl_set(game, GGTL_PLY_LIM, ply1);
			}
			else {
				ggtl_set(game, GGTL_PLY_TIMELIM, ply2);
				ggtl_set(game, GGTL_PLY_LIM, ply2);
			}
		}

		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
                else if (!strncmp(move, "rate", 4)) {
			int ply = ggtl_get(game, GGTL_PLY_LAST);
			if (ply < 1) ply = ggtl_get(game, GGTL_PLY_LIM);

                        printf("minimax value: %d\n\n", ggtl_rate_move(game, ply));
                        board = NULL;
                }
                else if (!strncmp(move, "redisp", 6)) {
                        board = ggtl_peek_pos(game);
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
			struct ggtl_move mv;
			mv.x = move[1] - '0';
			mv.y = move[0] - '0';
			board = ggtl_move(game, &mv);
			
			if (!board) {

#if cfg__moth_iterative
				board = ggtl_alphabeta_iterative(game);
#else
				board = ggtl_alphabeta(game);
#endif
				if (board) {
					printf("searched to ply %d\n",
						ggtl_get(game, GGTL_PLY_LAST));
				}
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
	struct ggtl_pos initial = {{{0}}, 1};
	int ply1 = 1, ply2 = 1;

	initial.b[3][4] = initial.b[4][3] = 1;
	initial.b[3][3] = initial.b[4][4] = 2;

	greeting();

	game = ggtl_new(make_move, end_of_game, find_moves, evaluate);
	if (!ggtl_init(game, &initial, sizeof initial, sizeof(struct ggtl_move))) {
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

	game = mainloop(game, ply1, ply2);
	ggtl_free(game);

	return 0;
}


