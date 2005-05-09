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

#include <ggtl/reversi.h>


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
static struct ggtl *mainloop(struct ggtl *game, int fixed, int ply1, int ply2)
{	
	char move[128] = {0};
	struct reversi_state *board;
	int score, ply;

	for (;;) {
		board = ggtl_peek_state(game);
		if (board) {
			reversi_state_draw(board);
		}
		if (ggtl_game_over(game)) {
			break;
		}
		printf("\nplayer %d (%c)\n", board->player, board->player==1?'-':'X');
		puts("Action (00-77|ENTER|undo|rate|redisp|save|load)?");


		if (board) 
			ply = board->player == 1 ? ply1 : ply2;

		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
#if 0
                else if (!strncmp(move, "rate", 4)) {
			int ply = ggtl_get(game, GGTL_PLY_LAST);
                        printf("minimax value: %d\n\n", ggtl_rate_move(game, ply));
                        board = NULL;
                }
#endif
                else if (!strncmp(move, "redisp", 6)) {
                        board = ggtl_peek_state(game);
                }
#if 0
		else if (!strncmp(move, "load", 4)) {
			struct ggtl *tmp;
			printf("Loading game, need a name: "); fflush(stdout);
			getline(move, sizeof move);
			if (move[0] && (tmp = resume(move))) {
				printf("loaded game from `%s'.", move);
				ggtl_free(game);
				game = tmp;
				board = ggtl_peek_state(game);
			}
			else {
				printf("failed loading game from `%s'.", move);
				board = NULL;
			}
		}
#endif
                else {
			struct reversi_move *mv;
			mv = reversi_move_new(move[1]-'0', move[0]-'0');
			board = ggtl_move(game, mv);
			
			if (!board) {
                                ggtl_ai_move(game);
			}
		}
	} 

#if 0
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
#endif
	return game;
}


int main(int argc, char **argv)
{
	struct ggtl *game;
	struct reversi_state *pos;
	int debug, fixed, level1, level2;

	getopts(argc, argv, &debug, &fixed, &level1, &level2);

        pos = reversi_state_new(8);

	game = reversi_init(ggtl_new(), pos);
	if (!game) {
		puts("sorry -- NO GAME FOR YOU!");
		return EXIT_FAILURE;
	}
	ggtl_ai_trace(game, debug);

	game = mainloop(game, fixed, level1, level2);
	ggtl_free(game);

	return 0;
}


