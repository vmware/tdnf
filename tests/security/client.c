#include "../../client/includes.h"
#include <assert.h>

static void
check_paths(const char *directory)
{
    char *path = NULL;
    const char *rejected[] = {"/absolute", "../outside", "sub/../../outside", ".", ""};

    for (size_t i = 0; i < sizeof(rejected) / sizeof(rejected[0]); i++)
    {
        assert(TDNFAppendPath(directory, rejected[i], &path) != 0);
        assert(path == NULL);
    }
    assert(TDNFAppendPath(directory, "repodata/./primary.xml", &path) == 0);
    assert(strstr(path, "/repodata/primary.xml") != NULL);
    free(path);
    path = NULL;

    /* An absent, relative destination is valid for reposync --urls. */
    assert(TDNFAppendPath("not-created", "repodata/primary.xml", &path) == 0);
    assert(path[0] == '/');
    free(path);
    path = NULL;
    assert(TDNFAppendPath("/", "repodata/primary.xml", &path) == 0);
    assert(strcmp(path, "/repodata/primary.xml") == 0);
    free(path);
    path = NULL;

    assert(chdir(directory) == 0);
    assert(mkdir("cache", 0700) == 0);
    assert(mkdir("cache-other", 0700) == 0);
    assert(TDNFAppendPath("cache", "../cache-other/file", &path) != 0);
    assert(path == NULL);
    assert(symlink("../cache-other", "cache/link") == 0);
    assert(TDNFAppendPath("cache", "link/file", &path) != 0);
    assert(path == NULL);
    assert(unlink("cache/link") == 0);
    assert(rmdir("cache") == 0);
    assert(rmdir("cache-other") == 0);
}

int
main(void)
{
    char directory[] = "/tmp/tdnf-security-client-XXXXXX";
    char *cwd = getcwd(NULL, 0);
    assert(cwd);
    assert(mkdtemp(directory));
    check_paths(directory);
    assert(chdir(cwd) == 0);
    free(cwd);
    assert(rmdir(directory) == 0);
    return 0;
}
