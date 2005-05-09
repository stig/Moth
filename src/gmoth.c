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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

#include <options/opt.h>
#include <ggtl/reversi.h>

#define XRES 400
#define YRES 400

struct ggtl *game;
enum { SIZE, LEVEL, TRACE, OPTCNT };
int values[OPTCNT] = {8, 2, -2};

void set(int what, int val)
{
        assert(what >= SIZE);
        assert(what < OPTCNT);
        values[what] = val;
}

int get(int what)
{ 
        assert(what >= SIZE);
        assert(what < OPTCNT);
        return values[what];
}


void getopts(int argc, char **argv)
{
        struct opt *opts;
        int help, error, size, trace, level;
        struct opt_defs options[] = {
                {"help", "h", 0, "0", "Print a help message and exit"},
                {"trace", "t", 1, "-2", "Print tracing information"},
                {"size", "s", 1, "8", "Size of board (default: 8)"},
                {"level", "l", 1, "2", "AI player level (default: 2)"},
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
        error |= opt_val(opts, "size", "int", &size);
        error |= opt_val(opts, "trace", "int", &trace);
        error |= opt_val(opts, "level", "int", &level);
        if (error) {
                fprintf(stderr, "Failure retrieving values. ");
                fprintf(stderr, "Last error was: %s\n", opt_strerror(error));
                exit(EXIT_FAILURE);
        }
        opt_free(opts);

        if (help) {
                opt_desc(options, 0);
                exit(EXIT_SUCCESS);
        }

        size += size % 2 ? 1 : 0;
        set(SIZE, size);
        set(TRACE, trace);
        set(LEVEL, level);
      }


/* 
 * Handle keyboard requensts.
 */
static void mykeyboard(unsigned char key, int x, int y)
{
        int level = get(LEVEL);
        int trace = get(TRACE);

        ggtl_ai_trace(game, trace);
        ggtl_ai_level(game, level);

        switch(key) {
                case 'l':
                        set(LEVEL, level + 1);
                        break;

                case 'L':
                        if (level > 1) { set(LEVEL, level - 1); }
                        break;

                case 't':
                        set(TRACE, trace + 1);
                        break;

                case 'T':
                        if (trace > -2) { set(TRACE, trace - 1); }
                        break;

                case 'u':
                        (void)ggtl_undo(game);
                        break;

                case ' ':
                        ggtl_ai_move(game);
                        break;

                case 'q': 
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

        if (state == GLUT_DOWN) {
                void *moved = NULL;
                if (button == GLUT_LEFT_BUTTON) {
                        struct reversi_move *mv;
                        int size = get(SIZE);

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
static void drawdisc(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
        int i;
        GLdouble rx = (x2 - x1) / 2.0;
        GLdouble ry = (y2 - y1) / 2.0;
        GLdouble x = x1 + rx;
        GLdouble y = y1 + ry;

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
static void drawgrid(GLdouble width, GLdouble height)
{
        int i;
        int size = get(SIZE);
        GLdouble x_step = width / size;
        GLdouble y_step = height / size;

        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        for (i = 1; i <= size; i++) {
                glVertex2d(0.0, y_step * i);
                glVertex2d(width, y_step * i);
                glVertex2d(x_step * i, 0.0);
                glVertex2d(x_step * i, height);
        }
        glEnd();
}


/* 
 * Draw a state; wizz through an array and draw discs in the correct
 * colour when needed.
 */
static void drawstate(struct reversi_state *board, GLdouble width, GLdouble height)
{
        int size = get(SIZE);
        GLdouble x_step = width / (GLdouble)size;
        GLdouble y_step = height / (GLdouble)size;
        int i, j, c;

        for (i = 0; i < size; i++) {
                for (j = 0; j < size; j++) {
                        c = board->board[i][j];
                        if (c) {
                                if (c == 1) glColor3f(1.0, 1.0, 1.0);
                                else glColor3f(0.0, 0.0, 0.0);

                                drawdisc(
                                        x_step * (GLdouble)i,
                                        y_step * (GLdouble)j,
                                        x_step * (GLdouble)(i+1), 
                                        y_step * (GLdouble)(j+1)
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
        int width = glutGet(GLUT_WINDOW_WIDTH);
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        struct reversi_state *board = ggtl_peek_state(game);

        if (ggtl_game_over(game)) 
                gameover(board);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.0, 0.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        gluOrtho2D(0.0, (GLdouble)width, 0.0, (GLdouble)height);

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

        getopts(argc, argv);

        pos = reversi_state_new(get(SIZE));
        game = reversi_init(ggtl_new(), pos);

        if (!game) {
                puts("sorry -- NO GAME FOR YOU!");
                return EXIT_FAILURE;
        }

        glutMainLoop();
        return 0;
}

