
#include <stdio.h>
#include <stdlib.h>
#include <options/opt.h>
#include "common.h"

void getopts(int argc, char **argv, int *debug, int *fixed, int *level1, int *level2)
{
	struct opt *opts;
	int help, longhelp, error;
	struct opt_defs options[] = {
		{"help", "h", 0, "0",	"Print a short help message and exit"},
		{"longhelp", "H", 0, "0","Print help with default values and exit"},
		{"debug", "d", 1, "-2",	"Print debug level messages"},
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

/* arch-tag: 8b7fe550-f212-4b6e-8a60-113f4b548a20
 */
