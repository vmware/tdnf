/* Exercise the private diagnostic helper without exporting it from libtdnf. */
#include "../../solv/tdnfpackage.c"
#include <assert.h>

int
main(void)
{
    SolvSack sack = {0};
    char previous[256] = {0};
    char dependency[257] = {0};
    char problem[320] = {0};

    /* Both accepted boundaries must preserve a complete, terminated name. */
    for (size_t length = 254; length <= 256; length++)
    {
        memset(dependency, 'a', length);
        dependency[length] = '\0';
        memset(previous, 0, sizeof(previous));
        if (length < sizeof(previous))
            memcpy(previous, dependency, length);
        assert(snprintf(problem, sizeof(problem), "package requires %s, unavailable",
                        dependency) > 0);
        assert(check_for_providers(&sack, SOLVER_RULE_PKG_REQUIRES,
                                   problem, previous) ==
               (length < sizeof(previous) ? 0 : ERROR_TDNF_SOLV_FAILED));
        if (length == sizeof(previous))
            assert(previous[0] == '\0');
        else
            assert(strcmp(previous, dependency) == 0);
    }
    return 0;
}
