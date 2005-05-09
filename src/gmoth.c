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
        struct reversi_state *s = ggtl_peek_state(game);
        int size = s->size;

        if (state == GLUT_DOWN) {
                void *moved = NULL;
                if (button == GLUT_LEFT_BUTTON) {
                        struct reversi_move *mv;

                        mv = reversi_move_new(
                          x / (width / size), 
                          (size - 1) - y / (height / size)
                        );
                        moved = ggtl_move(game, mv);
                }
                else if (button == GLUT_RIGHT_BUTTON) {
                        moved = ggtl_ai_move(game);
                }

                if (moved) { 
                        glutPostRedisplay();
                }
        }
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
        struct reversi_state *state = ggtl_peek_state(game);
        int size = state->size;
        GLfloat x_step = width / (GLfloat)size;
        GLfloat y_step = height / (GLfloat)size;

        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        for (i = 0; i < size+1; i++) {
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
        int size = board->size;
        int x_step = width / size;
        int y_step = height / size;
        int i, j, c;

        for (i = 0; i < size; i++) {
                for (j = 0; j < size; j++) {
                        c = board->board[i][j];
                        if (c) {
                                if (c == 1) glColor3f(1.0, 1.0, 1.0);
                                else glColor3f(0.0, 0.0, 0.0);

                                drawdisc(
                                        x_step * i,
                                        y_step * j,
                                        x_step * (i+1), 
                                        y_step * (j+1)
                                );
                        }
                }
        }
}


static void gameover(struct reversi_state *board)
{
        int score = ggtl_eval(game);
        int player = board->player;

        if (!score) {
                puts("The game ended in a draw\n\n");
        }
        else {
                printf("Player %d %s\n\n", player, 
                        score > 0 ?  "won" : "lost");
        }
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

        reversi_state_draw(board);
        if (ggtl_game_over(game)) 
                gameover(board);

        drawgrid(width, height);
        drawstate(board, width, height);

        glutSwapBuffers();
        glFlush();
}


int main(int argc, char **argv)
{
        struct reversi_state *pos;
        int debug, size;

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(XRES, YRES);
        glutCreateWindow("gMoth");

        glutKeyboardFunc(mykeyboard);
        glutDisplayFunc(mydisplay);
        glutMouseFunc(mymouse);

        getopts(argc, argv, &size, &debug, &fixed, &ply1, &ply2);

        size += size % 2 ? 1 : 0;
        pos = reversi_state_new(size);
        game = reversi_init(ggtl_new(), pos);

        if (!game) {
                puts("sorry -- NO GAME FOR YOU!");
                return EXIT_FAILURE;
        }
        ggtl_ai_trace(game, debug);

        glutMainLoop();
        return 0;
}

