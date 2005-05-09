/* 
 * gconnect4 -- the classic game
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
#include <config-options.h>
#include <connect4/libc4.h>

enum {
	XRES = 400,
	YRES = 300
};

struct ggtl *game;
int fixed, ply, ply1, ply2;


/* 
 * Handle keyboard requensts.
 */
static void mykeyboard(unsigned char key, int x, int y)
{
	struct ggtl *tmp;
	char filename[] = "gconnect.savegame";
	int lply = ggtl_get(game, GGTL_PLY_LAST);

	if (lply < 1) 
		lply = ply;

	switch(key) {
		case 'l':
		case 'L':
			if ((tmp = resume(filename))) {
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
                        printf("minimax value: %d\n\n", ggtl_rate_move(game, lply));
			break;

		case 's':
		case 'S':
			if (!save(filename, game))
				puts("success");
			else puts("failed");
			break;
			
		case 'u':
		case 'U':
			(void)ggtl_undo(game);
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

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			struct ggtl_move *move = ggtl_pop_move(game);
			if (!move)
				move = ensure_move();
			move->col = x / (width / COLS);
			(void)ggtl_move(game, move);
		}
			
		if (button == GLUT_RIGHT_BUTTON) {

			if (fixed)
				(void)ggtl_alphabeta(game, ply);
			else
				(void)ggtl_alphabeta_iterative(game, ply);
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
	GLfloat x_step = (GLfloat)width / COLS;
	GLfloat y_step = (GLfloat)height / ROWS;

	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	for (i = 0; i < COLS+1; i++) {
		glVertex2f(x_step * i, 0.0);
		glVertex2f(x_step * i, (GLfloat)height);
	}
	for (i = 0; i < ROWS+1; i++) {
		glVertex2f(0.0, y_step * i);
		glVertex2f((GLfloat)width, y_step * i);
	}
	glEnd();
}


/* 
 * Draw a state; wizz through an array and draw discs in the correct
 * colour when needed.
 */
static void drawstate(struct ggtl_pos *b, int width, int height)
{
	int i, j, c;
	int x_step = width / COLS;
	int y_step = height / ROWS;

	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			c = b->b[i][j];
			if (c) {
				int row = ROWS - i - 1;

				if (c == 1) 
					glColor3f(1.0, 0.0, 0.0);
				else 
					glColor3f(1.0, 1.0, 0.0);

				drawdisc(x_step * j, y_step * row, x_step * (j+1), 
						y_step * (row+1));
			}
		}
	}
}


static void mydisplay(void)
{
	struct ggtl_pos *board = ggtl_peek_pos(game);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);


	if (board)
		ply = board->player == 1 ? ply1 : ply2;

	drawgrid(width, height);
	drawstate(board, width, height);

	glutSwapBuffers();
	glFlush();

	if (end_of_game(board)) {
		display(board);
		gameover(board);
		ggtl_free(game);
		exit(EXIT_SUCCESS);
	}
}


int main(int argc, char **argv)
{
	struct ggtl_pos *pos, start = {NULL, {{0}}, 1};
	int level1, level2, fixed, debug;

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(XRES, YRES);
        glutCreateWindow("gConnect4");

        glutKeyboardFunc(mykeyboard);
        glutDisplayFunc(mydisplay);
        glutMouseFunc(mymouse);

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

        glutMainLoop();
	return 0;
}

/* arch-tag: Stig Brautaset Mon Apr 14 13:11:05 BST 2003
 */
