/* Exercise the private diagnostic helper without exporting it from libtdnf. */
#include "../../solv/tdnfpackage.c"
#include <assert.h>

int
main(void)
{
    SolvSack sack = {0};
    const size_t lengths[] = {1, 255, 256, 257, 4096};

    /* Names of any length must be extracted completely, so that they match
     * the cached previous name instead of being truncated or rejected. */
    for (size_t n = 0; n < sizeof(lengths) / sizeof(lengths[0]); n++)
    {
        size_t length = lengths[n];
        char *dependency = calloc(1, length + 1);
        char *problem = NULL;
        char *previous = NULL;

        assert(dependency);
        memset(dependency, 'a', length);
        assert(asprintf(&problem, "package requires %s, unavailable",
                        dependency) > 0);
        previous = strdup(dependency);
        assert(previous);
        assert(check_for_providers(&sack, SOLVER_RULE_PKG_REQUIRES,
                                   problem, &previous) == 0);
        assert(previous && strcmp(previous, dependency) == 0);
        free(previous);
        free(problem);
        free(dependency);
    }
    return 0;
}
