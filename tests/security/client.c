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

static void
check_hash_parameters(void)
{
    hash_op rejected[] = {
        {"md5", 16},
        {NULL, SHA256_DIGEST_LENGTH},
        {"", SHA256_DIGEST_LENGTH},
        {"sha256", SHA256_DIGEST_LENGTH - 1},
        {"sha256", SHA256_DIGEST_LENGTH + 1}
    };
    const int invalid_types[] = {-1, 0, TDNF_HASH_SENTINEL};
    uint8_t digest[EVP_MAX_MD_SIZE] = {0};

    /* Invalid requests must fail before opening this absent file. */
    assert(access("missing-hash-file", F_OK) == -1 && errno == ENOENT);
    for (size_t i = 0; i < sizeof(rejected) / sizeof(rejected[0]); i++)
    {
        assert(TDNFGetDigestForFile("missing-hash-file", &rejected[i],
                                    digest) == ERROR_TDNF_INVALID_PARAMETER);
    }
    for (size_t i = 0;
         i < sizeof(invalid_types) / sizeof(invalid_types[0]); i++)
    {
        assert(TDNFCheckHash("missing-hash-file", digest, invalid_types[i])
               == ERROR_TDNF_INVALID_PARAMETER);
    }
}

static void
check_package_checksums(void)
{
    const int types[] = {REPOKEY_TYPE_MD5, REPOKEY_TYPE_SHA256, 0};
    unsigned char digest[EVP_MAX_MD_SIZE] = {0};

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
    {
        SolvSack sack = {0};
        SolvPackageList packages = {0};
        PTDNF_PKG_INFO info = NULL;
        Repo *repo;
        Repodata *data;
        Solvable *solvable;
        Id package;
        uint32_t result;

        sack.pPool = pool_create();
        assert(sack.pPool);
        repo = repo_create(sack.pPool, "checksum-test");
        assert(repo);
        package = repo_add_solvable(repo);
        solvable = pool_id2solvable(sack.pPool, package);
        solvable->name = pool_str2id(sack.pPool, "checksum-test", 1);
        solvable->evr = pool_str2id(sack.pPool, "1-1", 1);
        solvable->arch = pool_str2id(sack.pPool, "noarch", 1);
        data = repo_add_repodata(repo, 0);
        assert(data);
        repodata_set_str(data, package, SOLVABLE_SUMMARY, "Checksum test");
        repodata_set_location(data, package, 0, NULL, "checksum-test.rpm");
        if (types[i])
        {
            /* Store test metadata only; never compute an MD5 digest. */
            repodata_set_bin_checksum(data, package, SOLVABLE_CHECKSUM,
                                      types[i], digest);
        }
        repodata_internalize(data);
        queue_init(&packages.queuePackages);
        queue_push(&packages.queuePackages, package);

        result = TDNFPopulatePkgInfos(&sack, &packages, &info);
        if (types[i] == REPOKEY_TYPE_MD5)
        {
            assert(result == ERROR_TDNF_INVALID_REPO_FILE);
            assert(info == NULL);
        }
        else
        {
            assert(result == 0 && info);
            if (types[i])
            {
                assert(info->nChecksumType == TDNF_HASH_SHA256);
                assert(info->pbChecksum);
                assert(memcmp(info->pbChecksum, digest,
                              SHA256_DIGEST_LENGTH) == 0);
            }
            else
            {
                assert(info->pbChecksum == NULL);
            }
            TDNFFreePackageInfo(info);
        }
        queue_free(&packages.queuePackages);
        pool_free(sack.pPool);
    }
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
    check_hash_parameters();
    check_package_checksums();
    assert(chdir(cwd) == 0);
    free(cwd);
    assert(rmdir(directory) == 0);
    return 0;
}
