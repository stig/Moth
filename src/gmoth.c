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
#include <GLUT/glut.h>

#include <ggtl/reversi.h>

#define XRES 400
#define YRES 400


struct ggtl *game;
int ply1, ply2, fixed;


/* 
 * Handle keyboard requensts.
 */
static void mykeyboard(unsigned char key, int x, int y)
{
	ggtl_ai_level(game, 2);

	switch(key) {
		case 'u':
		case 'U':
			(void)ggtl_undo(game);
			break;

		case ' ':
		case 'm':
		case 'M':
			ggtl_ai_move(game);
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
static void mymouse(int button, int state, int x, int y)
{
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	struct reversi_state *pos = ggtl_peek_state(game);

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			struct reversi_move *mv;

                        mv = reversi_move_new(
                          x / (width / 8), 
                          7 - y / (height / 8)
                        );
                        ggtl_move(game, mv);
		}
			
		if (button == GLUT_RIGHT_BUTTON) {
                        ggtl_ai_move(game);
		}
	}

	glutPostRedisplay();
}


/*
 * Draw a disc in the quadrant specified
 */
static void drawdisc(int x1, int y1, int x2, int y2)
{
	int i;
	int rx = (x2 - x1) / 2;
	int ry = (y2 - y1) / 2;
	int x = x1 + rx;
	int y = y1 + ry;

	glBegin(GL_POLYGON);
	for (i = 0; i < 360; i++) {
		GLfloat xt = x + rx * cos(i / 57.0);
		GLfloat xy = y + ry * sin(i / 57.0);
		glVertex2f(xt, xy);
	}
	glEnd();
}


/* 
 * Draw the grid
 */
static void drawgrid(int width, int height)
{
	int i;
	GLfloat x_step = width / 8.0;
	GLfloat y_step = height / 8.0;

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	for (i = 0; i < 9; i++) {
		glVertex2f(0.0, y_step * i);
		glVertex2f((GLfloat)width, y_step * i);
		glVertex2f(x_step * i, 0.0);
		glVertex2f(x_step * i, (GLfloat)height);
	}
	glEnd();
}


/* 
 * Draw a state; wizz through an array and draw discs in the correct
 * colour when needed.
 */
static void drawstate(struct reversi_state *board, int width, int height)
{
	int i, j, c;
	int x_step = width / 8;
	int y_step = height / 8;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			c = board->board[i][j];
			if (c) {
				if (c == 1) glColor3f(1.0, 1.0, 1.0);
				else glColor3f(0.0, 0.0, 0.0);

				drawdisc(x_step * i, y_step * j, x_step * (i+1), y_step * (j+1));
			}
		}
	}
}


static void gameover(struct reversi_state *board)
{
	int score = ggtl_eval(game);
	int player = board->player;

	reversi_state_draw(board);

	if (score > 0) {
		printf("Player %d won, with a margin of %d\n\n", player, score);
	}
	else if (score < 0) {
		printf("Player %d won, with a margin of %d\n\n", player, -score);
	}
	else {
		puts("The game ended in a draw\n\n");
	}
	ggtl_free(game);
	exit(EXIT_SUCCESS);
}


static void mydisplay(void)
{
	int ply;
	struct reversi_state *board = ggtl_peek_state(game);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);

	if (ggtl_game_over(game)) 
		gameover(board);
  /* reversi_state_draw(board); */

	drawgrid(width, height);
	drawstate(board, width, height);

	glutSwapBuffers();
	glFlush();
}


int main(int argc, char **argv)
{
	struct reversi_state *pos;
	int debug;

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(XRES, YRES);
        glutCreateWindow("gMoth");

        glutKeyboardFunc(mykeyboard);
        glutDisplayFunc(mydisplay);
        glutMouseFunc(mymouse);

	getopts(argc, argv, &debug, &fixed, &ply1, &ply2);

        pos = reversi_state_new(8);
	game = reversi_init(ggtl_new(), pos);

	if (!game) {
		puts("sorry -- NO GAME FOR YOU!");
		return EXIT_FAILURE;
	}
	ggtl_ai_trace(game, debug);

        glutMainLoop();
	return 0;
}

