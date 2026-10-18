#include "../../client/includes.h"
#include <assert.h>
#include <glob.h>

static void
write_file(const char *path, const char *text)
{
    FILE *file = fopen(path, "w");
    assert(file);
    assert(fputs(text, file) >= 0);
    assert(fclose(file) == 0);
}

static void
check_file(const char *path, const char *text)
{
    char buffer[64] = {0};
    FILE *file = fopen(path, "r");
    assert(file);
    assert(fread(buffer, 1, sizeof(buffer) - 1, file) == strlen(text));
    assert(strcmp(buffer, text) == 0);
    assert(fclose(file) == 0);
}

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

static void
check_downloads(const char *directory)
{
    TDNF handle = {0};
    TDNF_CONF conf = {0};
    TDNF_CMD_ARGS args = {0};
    TDNF_REPO_DATA repo = {0};
    char *url = NULL;
    struct stat st = {0};
    glob_t matches = {0};

    handle.pConf = &conf;
    handle.pArgs = &args;
    args.nQuiet = 1;
    repo.nRetries = 1;
    write_file("source", "download contents");
    write_file("destination.tmp", "existing temporary file");
    assert(TDNFAllocateStringPrintf(&url, "file://%s/source", directory) == 0);
    assert(TDNFDownloadFile(&handle, &repo, url, "destination", NULL) == 0);
    check_file("destination", "download contents");
    check_file("destination.tmp", "existing temporary file");
    assert(stat("destination", &st) == 0);
    assert((st.st_mode & 0777) == 0644);
    assert(glob("destination.tmp.*", 0, NULL, &matches) == GLOB_NOMATCH);
    globfree(&matches);
    free(url);

    assert(TDNFAllocateStringPrintf(&url, "file://%s/missing", directory) == 0);
    assert(TDNFDownloadFile(&handle, &repo, url, "destination", NULL) != 0);
    check_file("destination", "download contents");
    check_file("destination.tmp", "existing temporary file");
    assert(glob("destination.tmp.*", 0, NULL, &matches) == GLOB_NOMATCH);
    globfree(&matches);
    free(url);
    assert(unlink("source") == 0);
    assert(unlink("destination") == 0);
    assert(unlink("destination.tmp") == 0);
}

int
main(void)
{
    char directory[] = "/tmp/tdnf-security-client-XXXXXX";
    char *cwd = getcwd(NULL, 0);
    assert(cwd);
    assert(mkdtemp(directory));
    check_paths(directory);
    check_downloads(directory);
    assert(chdir(cwd) == 0);
    free(cwd);
    assert(rmdir(directory) == 0);
    return 0;
}
