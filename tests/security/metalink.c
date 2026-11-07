#include "../../plugins/metalink/includes.h"
#include <assert.h>

static void
check_parse(const char *document, size_t length, int valid)
{
    TDNF_ML_CTX *context = calloc(1, sizeof(*context));
    FILE *file = tmpfile();
    uint32_t result;
    assert(context && file);
    assert(fwrite(document, 1, length, file) == length);
    rewind(file);
    result = TDNFMetalinkParseFile(context, file, "repomd.xml");
    assert(valid ? result == 0 : result != 0);
    if (valid)
    {
        TDNF_ML_URL_INFO *url;
        assert(context->urls);
        url = context->urls->data;
        assert(strcmp(url->url, "https://example.invalid/repomd.xml") == 0);
    }
    TDNFMetalinkFree(context);
    assert(fclose(file) == 0);
}

static void
check_xml(void)
{
    /* Valid preferences first, invalid ones after them. */
    const char *preferences[] = {"0", "100", "0100",
                                 "101", "-1", "+5", " 5", "5 ", "1junk", "",
                                 "999999999999999999999999"};
    const size_t nValid = 3;
    char document[512];
    int length;

    for (size_t i = 0; i < sizeof(preferences) / sizeof(preferences[0]); i++)
    {
        length = snprintf(document, sizeof(document),
            "<metalink><file name=\"repomd.xml\">\n<url protocol=\"https\" preference=\"%s\">"
            "https://example.invalid/repomd.xml</url></file></metalink>", preferences[i]);
        assert(length > 0 && (size_t)length < sizeof(document));
        check_parse(document, (size_t)length, i < nValid);
    }
    length = snprintf(document, sizeof(document),
        "<metalink><file name=\"repomd.xml\">\n<url protocol=\"https\">"
        "https://example.invalid/repomd.xml</url></file></metalink>");
    assert(length > 0 && (size_t)length < sizeof(document));
    check_parse(document, (size_t)length, 1);
    check_parse(document, (size_t)length - 1, 0);
    check_parse(document, (size_t)length + 1, 0); /* Embedded NUL is XML input. */
    check_parse("", 0, 0);
}

static void
check_parse_error(const char *document, uint32_t expected)
{
    TDNF_ML_CTX *context = calloc(1, sizeof(*context));
    FILE *file = tmpfile();
    assert(context && file);
    assert(fputs(document, file) >= 0);
    rewind(file);
    assert(TDNFMetalinkParseFile(context, file, "repomd.xml") == expected);
    TDNFMetalinkFree(context);
    assert(fclose(file) == 0);
}

static void
check_errors(void)
{
    /* The first handler error is returned, not replaced by later callbacks
     * or by the parse failure caused by stopping the parser. */
    check_parse_error(
        "<metalink><file name=\"repomd.xml\">\n"
        "<verification><hash>0123</hash></verification>\n"
        "<url protocol=\"https\">https://example.invalid/repomd.xml</url>\n"
        "</file></metalink>",
        ERROR_TDNF_METALINK_PARSER_MISSING_HASH_ATTR);
    check_parse_error(
        "<metalink><file name=\"other.xml\">\n"
        "<url protocol=\"https\">https://example.invalid/repomd.xml</url>\n"
        "</file></metalink>",
        ERROR_TDNF_METALINK_PARSER_INVALID_FILE_NAME);
    /* Documents that are not well-formed fail as invalid documents. */
    check_parse_error(
        "<metalink><file name=\"repomd.xml\">\n"
        "<url protocol=\"https\">https://example.invalid/repomd.xml</file>",
        ERROR_TDNF_METALINK_PARSER_INVALID_DOC_OBJECT);
}

static void
check_hashes(void)
{
    char filename[] = "/tmp/tdnf-security-hash-XXXXXX";
    const char *correct = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    char mismatch[65];
    char removed_digest[] = "d41d8cd98f00b204e9800998ecf8427e";
    TDNF_ML_HASH_INFO strongest = {0};
    TDNF_ML_HASH_INFO weaker = {0};
    TDNF_ML_HASH_LIST tail = {NULL, &weaker};
    TDNF_ML_HASH_LIST head = {&tail, &strongest};
    TDNF_ML_CTX context = {0};
    int fd = mkstemp(filename);
    assert(fd >= 0);
    assert(close(fd) == 0);
    context.hashes = &head;
    memset(mismatch, '0', sizeof(mismatch) - 1);
    mismatch[sizeof(mismatch) - 1] = '\0';
    assert(TDNFAllocateString("sha256", &strongest.type) == 0);
    strongest.value = mismatch;
    assert(TDNFAllocateString("sha1", &weaker.type) == 0);
    assert(TDNFAllocateString("da39a3ee5e6b4b0d3255bfef95601890afd80709", &weaker.value) == 0);
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == ERROR_TDNF_CHECKSUM_VALIDATION_FAILED);
    free(weaker.type);
    assert(TDNFAllocateString("unsupported", &weaker.type) == 0);
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == ERROR_TDNF_CHECKSUM_VALIDATION_FAILED);
    strongest.value = NULL;
    assert(TDNFAllocateString("bad", &strongest.value) == 0);
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == ERROR_TDNF_CHECKSUM_VALIDATION_FAILED);
    free(strongest.value);
    assert(TDNFAllocateString(correct, &strongest.value) == 0);
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == 0);
    free(weaker.type);
    free(weaker.value);
    assert(TDNFAllocateString("sha256", &weaker.type) == 0);
    assert(TDNFAllocateString(correct, &weaker.value) == 0);
    free(strongest.value);
    strongest.value = mismatch;
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == 0);

    /* A removed algorithm cannot validate a file, but does not prevent a
     * supported SHA checksum in the same metalink from being checked. */
    free(strongest.type);
    assert(TDNFAllocateString("md5", &strongest.type) == 0);
    strongest.value = removed_digest;
    head.next = NULL;
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == ERROR_TDNF_INVALID_REPO_FILE);
    head.next = &tail;
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) == 0);
    context.hashes = NULL;
    assert(TDNFCheckRepoMDFileHashFromMetalink(filename, &context) != 0);
    free(strongest.type);
    free(weaker.type);
    free(weaker.value);
    assert(unlink(filename) == 0);
}

int
main(void)
{
    check_xml();
    check_errors();
    check_hashes();
    return 0;
}
