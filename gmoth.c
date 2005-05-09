/* 
 * gmoth -- an Othello game (gui version)
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "moth/moth-common.h"

#define XRES 400
#define YRES 400


struct ggtl *game;
int ply1, ply2;


/* 
 * Handle keyboard requensts.
 */
void mykeyboard(unsigned char key, int x, int y)
{
	struct ggtl *tmp;
	char filename[] = "gmoth.savegame";

	switch(key) {
		case 'l':
		case 'L':
			tmp = ggtl_new(make_move, end_of_game, find_moves, evaluate);
			if (tmp && ggtl_resume(tmp, filename)) {
				printf("loaded game from `%s'.", filename);
				ggtl_free(game);
				game = tmp;
			}
			else {
				printf("failed loading game from `%s'.", filename);
				ggtl_free(tmp);
			}
			break;

		case 'r':
		case 'R':
                        printf("minimax value: %d\n\n", ggtl_rate_move(game));
			break;

		case 's':
		case 'S':
			if (ggtl_save(game, filename))
				puts("success");
			else puts("failed");
			break;
			
		case 'u':
		case 'U':
			ggtl_undo(game);
			break;

		case 'q': 
		case 'Q':
			ggtl_free(game);
			exit(EXIT_SUCCESS);
			break;

		default:
			break;
	}

	glutPostRedisplay();
}


/* 
 * Handle mouse requests
 */
void mymouse(int button, int state, int x, int y)
{
	char move[2];
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	move[0] = 7 - y / (height / 8);
	move[1] = x / (width / 8);

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			ggtl_move(game, move);
		}
			
		if (button == GLUT_RIGHT_BUTTON) {
			ggtl_alphabeta_iterative(game);
		}
	}

	glutPostRedisplay();
}


/*
 * Draw a disc in the quadrant (and with the colour) specified
 */
void drawdisc(int x1, int y1, int x2, int y2, float col)
{
	int i;
	float rx = (x2 - x1) / 2.0;
	float ry = (y2 - y1) / 2.0;
	float x = x1 + rx;
	float y = y1 + ry;

	glColor3f(col, col, col);
	glBegin(GL_POLYGON);
	for (i = 0; i < 360; i++) {
		float xt = x + rx * cos(i / 57.0);
		float xy = y + ry * sin(i / 57.0);
		glVertex2f(xt, xy);
	}
	glEnd();
}


/* 
 * Draw the grid
 */
void drawgrid(int width, int height)
{
	int i;
	int x_step = width / 8;
	int y_step = height / 8;

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	for (i = 0; i < 9; i++) {
		glVertex2f(0, y_step * i);
		glVertex2f(width, y_step * i);
		glVertex2f(x_step * i, 0);
		glVertex2f(x_step * i, height);
	}
	glEnd();
}


/* 
 * Draw a state; wizz through an array and draw discs in the correct
 * colour when needed.
 */
void drawstate(const char *board, int width, int height)
{
	int i, j, c;
	int x_step = width / 8;
	int y_step = height / 8;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = a(board, i, j);
			if (c == 1) {
				drawdisc(x_step * i, y_step * j, x_step * (i+1), y_step * (j+1), 1.0);
			}
			else if (c == 2) {
				drawdisc(x_step * i, y_step * j, x_step * (i+1), y_step * (j+1), 0.0);
			}
		}
	}
}

void gameover(const char *board)
{
	int score;

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
	ggtl_free(game);
}

void mydisplay(void)
{
	const char *board = ggtl_peek_state(game);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	int i, player;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	gluOrtho2D(0, width, 0, height);

	if (end_of_game(board, 1) && end_of_game(board, 2)) {
		gameover(board);
	}

	player = ggtl_get(game, GGTL_PLAYER_TURN); 
	if (player == 1) {
		ggtl_set(game, GGTL_PLY_TIMELIM, ply1);
	}
	else {
		ggtl_set(game, GGTL_PLY_TIMELIM, ply2);
	}

	drawgrid(width, height);
	drawstate(board, width, height);

	glutSwapBuffers();
	glFlush();
}


int main(int argc, char **argv)
{
	char board[8][8] = {{0}};
	int ply1 = 30, ply2 = 30;

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(XRES, YRES);
        glutCreateWindow("gMoth");

	greeting();

	board[3][4] = board[4][3] = 1;
	board[3][3] = board[4][4] = 2;
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

        glutKeyboardFunc(mykeyboard);
        glutDisplayFunc(mydisplay);
        glutMouseFunc(mymouse);

        glutMainLoop();
	return 0;
}

