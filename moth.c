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



/* 
 * Draw a game position on screen.
 */
static void display(const void *boarddata)
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
 * This function actually plays the game.
 */
static struct ggtl *mainloop(struct ggtl *game, int ply1, int ply2)
{	
	char move[128] = {0};
	const void *board;
	int score, player;

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
			ggtl_set(game, GGTL_PLY_TIMELIM, ply1);
		}
		else {
			ggtl_set(game, GGTL_PLY_TIMELIM, ply2);
		}

		printf("\nplayer %d (%c)\n", player, player==1?'-':'#');
		printf("Chose action (00-77|ENTER|undo|rate|redisp|save|load): ");
		fflush(stdout);
		getline(move, sizeof move);

		if (!strncmp(move, "undo", 4)) {
			board = ggtl_undo(game);
                        if (!board) {
                                puts("Error: no move to undo\n");
                        }
                }
                else if (!strncmp(move, "rate", 4)) {
                        printf("minimax value: %d\n\n", ggtl_rate_move(game));
                        board = NULL;
                }
                else if (!strncmp(move, "redisp", 6)) {
                        board = ggtl_peek_state(game);
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
				board = ggtl_alphabeta_iterative(game);
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
	char board[8][8] = {{0}};
	int ply1 = 30, ply2 = 30;

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

	game = mainloop(game, ply1, ply2);
	ggtl_free(game);

	return 0;
}


