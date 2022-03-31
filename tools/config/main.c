#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <glob.h>

#include "../../common/config.h"

#include "../../llconf/nodes.h"
#include "../../llconf/modules.h"
#include "../../llconf/entry.h"
#include "../../llconf/ini.h"
#include "../../llconf/strutils.h"


#define ERR_CMDLINE     1
#define ERR_SYSTEM      2
#define ERR_NO_REPO     3
#define ERR_NO_SETTING  4
#define ERR_REPO_EXISTS 5

#define pr_err(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)

#define fail(rc, fmt, ...) { \
    pr_err(fmt, ##__VA_ARGS__); \
    exit(rc); \
}

#define check_cond(COND) if(!(COND)) { \
    pr_err("check_cond failed in %s line %d\n", \
        __FUNCTION__, __LINE__); \
    rc = -1; \
    ((void)(rc)); /* suppress "set but not used" warning */ \
    goto error; \
}

#define check_ptr(ptr) if(!(ptr)) { \
    pr_err("check_ptr failed in %s line %d\n", \
        __FUNCTION__, __LINE__); \
    rc = -1; \
    ((void)(rc)); /* suppress "set but not used" warning */ \
    goto error; \
}

#define safe_free(ptr) { if ((ptr) != NULL) { free(ptr); ptr = NULL; }}

struct cnfmodule *mod_ini = NULL;

static void
set_key_value(struct cnfnode *cn_repo, const char *keyval)
{
	const char *p = keyval;
    char key[256], *q = key;
    struct cnfnode *cn_keyval = NULL;

    /* parse key */
    while(*p &&
        (!isspace(*p) && (*p != '=')) &&
         q < key+sizeof(key)-1)
            *(q++) = *(p++);
    *q = 0;

    skip_spaces(&p);

    if(*p == '=') {
        p++;
        skip_spaces(&p);
        /* p is now pointing to the value */
        cn_keyval = find_child(cn_repo, key);
        if (cn_keyval == NULL) {
            cn_keyval = create_cnfnode(key);
            cnfnode_setval(cn_keyval, p);
            append_node(cn_repo, cn_keyval);
        } else {
            cnfnode_setval(cn_keyval, p);
        }
    } else
        fail(ERR_CMDLINE, "expected '=' after key %s\n", key);
}

static void
set_key_values(struct cnfnode *cn_root, const char *repo, char *kv[])
{
    struct cnfnode *cn_repo = find_child(cn_root, repo);
    if (cn_repo) {
        for (int i = 0; kv[i]; i++) {
            set_key_value(cn_repo, kv[i]);
        }
    }
}

static
void remove_keys(struct cnfnode *cn_root, const char *repo, char *keys[])
{
    struct cnfnode *cn_repo = find_child(cn_root, repo);
    if (cn_repo) {
        for (int i = 0; keys[i]; i++) {
            struct cnfnode *cn_keyval = find_child(cn_repo, keys[i]);
            if (cn_keyval) {
                unlink_node(cn_keyval);
                destroy_cnfnode(cn_keyval);
            } else
                fail(ERR_NO_SETTING, "key '%s' not found\n", keys[i]);
        }
    } else
        fail(ERR_NO_REPO, "repo '%s' not found\n", repo);
}

static
void remove_repo(struct cnfnode *cn_root, const char *repo)
{
    struct cnfnode *cn_repo = find_child(cn_root, repo);
    if (cn_repo) {
        unlink_node(cn_repo);
        destroy_cnfnode(cn_repo);
    } else
        fail(ERR_NO_REPO, "repo '%s' not found\n", repo);
}
static
char *get_repodir(const char *main_config)
{
    char *repodir = NULL;
    struct cnfnode *cn_root = cnfmodule_parse_file(mod_ini, main_config);

    if (cn_root) {
        struct cnfnode *cn_main = find_child(cn_root, "main");
        if (cn_main) {
            struct cnfnode *cn_repodir = find_child(cn_main, TDNF_CONF_KEY_REPODIR);
            if (cn_repodir) {
                repodir = strdup(cnfnode_getval(cn_repodir));
            }
        }
    }
    if (repodir == NULL)
        repodir = strdup(TDNF_DEFAULT_REPO_LOCATION);

    if (cn_root)
        destroy_cnftree(cn_root);

    return repodir;
}

static
struct cnfnode *find_repo(const char *repodir, const char *repo, char **pfilename)
{
    struct cnfnode *cn_root = NULL;
    char pattern[256];
    glob_t globbuf =  {0};
    int i, rc = 0;

    snprintf(pattern, sizeof(pattern), "%s/*.repo", repodir);
    rc = glob(pattern, 0, NULL, &globbuf);
    check_cond(rc == 0 || rc == GLOB_NOMATCH);
    if (rc == 0) {
        for (i = 0; globbuf.gl_pathv[i]; i++) {
            struct cnfnode *cn_repo = NULL;

            cn_root = cnfmodule_parse_file(mod_ini, globbuf.gl_pathv[i]);
            check_ptr(cn_root);
            cn_repo = find_child(cn_root, repo);
            if (cn_repo) {
                if (pfilename)
                    *pfilename = strdup(globbuf.gl_pathv[i]);
                break;
            }
            destroy_cnftree(cn_root);
            cn_root = NULL;
        }
    }

error:
    if (rc && cn_root) {
        destroy_cnftree(cn_root);
        cn_root = NULL;
    }
    globfree(&globbuf);
    return cn_root;
}

static
struct cnfnode *get_repo_root(const char *main_config, const char *repo, char **pfilename)
{
    struct cnfnode *cn_root = NULL;

    if (strcmp(repo, "main") == 0) {
        cn_root = cnfmodule_parse_file(mod_ini, main_config);
        if (cn_root == NULL)
            fail(ERR_SYSTEM, "could not parse config file %s\n", main_config);
        if (pfilename)
            *pfilename = strdup(main_config);
    } else {
        char *repodir = get_repodir(main_config);

        cn_root = find_repo(repodir, repo, pfilename);
        if (cn_root == NULL)
            fail(ERR_NO_REPO, "repo '%s' not found\n", repo);
    }
    return cn_root;
}

static
int write_file(struct cnfnode *cn_root, const char *filename)
{
    int rc = 0;
    char buf[256];

    snprintf(buf, sizeof(buf), "%s.tmp", filename);
    rc = cnfmodule_unparse_file(mod_ini, buf, cn_root);
    check_cond(rc == 0);

    rc = rename(buf, filename);
    check_cond(rc == 0);
error:
    return rc;
}

int main(int argc, char *argv[])
{
    char *main_config = TDNF_CONF_FILE;
    char *repo_config = NULL;

    int rc = 0;

    while(1) {
        int c;

        static struct option long_options[] = {
            {"config", 1, 0, 'c'},
            {"file", 1, 0, 'f'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "c:f:",
            long_options, NULL);

        if (c == -1)
            break;

        switch(c){
        case 'c':
            main_config = optarg;
            break;
        case 'f':
            repo_config = optarg;
            printf("repo_config=%s\n", repo_config);
            break;
        case '?':
        default:
            /* If it's an error, getopt has already produced an error message. */
            //usage();
            return 1;
        }
    }

    register_ini(NULL);
    mod_ini = find_cnfmodule("ini");

    /*
     * Process the action(s).
     */
    if (optind < argc) {
        int argcount = 0;
        char *action = NULL;
        char *repo = NULL;
        char *filename = NULL;
        struct cnfnode *cn_root = NULL;

        while (optind + argcount < argc)
            argcount++;

       	/*
        * Find the action.
        */
        action = argv[optind];
        if(strcmp(action, "edit") == 0 || strcmp(action, "create") == 0) {
            if (argcount < 2)
                fail(ERR_CMDLINE, "expected main or repo name\n");

            repo = argv[optind+1];

            if (argcount < 3)
                fail(ERR_CMDLINE, "Expected at least one setting.");

            if (strcmp(action, "edit") == 0)
                cn_root = get_repo_root(main_config, repo, &filename);
            else { /* create repo config */
                if(strcmp(repo, "main") != 0) {
                    char buf[256];
                    char *repodir = get_repodir(main_config);
                    if (!find_repo(repodir, repo, NULL)) {
                        struct cnfnode *cn_repo = create_cnfnode(repo);

                        if (repo_config == NULL) {
                            cn_root = create_cnfnode("(root)");
                            snprintf(buf, sizeof(buf), "%s/%s.repo", repodir, repo);
                            filename = strdup(buf);
                        } else {
                            cn_root = cnfmodule_parse_file(mod_ini, repo_config);
                            if (cn_root == NULL) {
                                if (errno == ENOENT)
                                    cn_root = create_cnfnode("(root)");
                                else
                                    fail(ERR_SYSTEM, "could not parse config file %s\n", repo_config);
                            }
                            filename = strdup(repo_config);
                        }
                        append_node(cn_root, cn_repo);

                    } else
                        fail(ERR_REPO_EXISTS, "repo '%s' already exists\n", repo);
                } else
                    fail(ERR_CMDLINE, "invalid repo name 'main'\n");
            }

            if (cn_root) {
                set_key_values(cn_root, repo, &argv[optind+2]);
                if (filename) {
                    rc = write_file(cn_root, filename);
                    if (rc != 0)
                        fail(ERR_SYSTEM, "failed to write file '%s': %s (%d)", filename, strerror(errno), errno);

                    safe_free(filename);
                } else
                    cnfmodule_unparse(mod_ini, stdout, cn_root);

                destroy_cnftree(cn_root);
            }
        } else if (strcmp(action, "get") == 0) {
            if (argcount < 2)
                fail(ERR_CMDLINE, "expected main or repo name\n");

            repo = argv[optind+1];

            if (argcount < 3)
                fail(ERR_CMDLINE, "expected one setting\n");

            cn_root = get_repo_root(main_config, repo, NULL);

            if (cn_root) {
                struct cnfnode *cn_repo = find_child(cn_root, repo);
                if (cn_repo) {
                    struct cnfnode *cn_keyval = find_child(cn_repo, argv[optind+2]);
                    if (cn_keyval)
                        printf("%s\n", cn_keyval->value);
                    else
                        fail(ERR_NO_SETTING, "'%s' not found in '%s'\n", argv[optind+2], repo);
                } else
                    fail(ERR_NO_REPO, "repo '%s' not found\n", repo);
                
                destroy_cnftree(cn_root);
            }
        } else if (strcmp(action, "remove") == 0) {
            if (argcount < 2)
                fail(ERR_CMDLINE, "expected main or repo name\n");

            repo = argv[optind+1];

            if (argcount < 3)
                fail(ERR_CMDLINE, "expected one setting\n");

            cn_root = get_repo_root(main_config, repo, &filename);

            if (cn_root) {
                remove_keys(cn_root, repo, &argv[optind+2]);
                if (filename) {
                    rc = write_file(cn_root, filename);
                    if (rc != 0)
                        fail(ERR_SYSTEM, "failed to write file '%s': %s (%d)", filename, strerror(errno), errno);

                    safe_free(filename);
                } else
                    cnfmodule_unparse(mod_ini, stdout, cn_root);

                destroy_cnftree(cn_root);
            }
        } else if (strcmp(action, "removerepo") == 0) {
            if (argcount < 2)
                fail(ERR_CMDLINE, "expected main or repo name\n");

            repo = argv[optind+1];

            cn_root = get_repo_root(main_config, repo, &filename);

            if (cn_root) {
                remove_repo(cn_root, repo);
                if (filename) {
                    if (cn_root->first_child) {
                        rc = write_file(cn_root, filename);
                        if (rc != 0)
                            fail(ERR_SYSTEM, "failed to write file '%s': %s (%d)", filename, strerror(errno), errno);
                    } else {
                        rc = unlink(filename);
                        if (rc != 0)
                             fail(ERR_SYSTEM, "failed to remove file '%s': %s (%d)", filename, strerror(errno), errno);
                    }
                    safe_free(filename);
                } else
                    cnfmodule_unparse(mod_ini, stdout, cn_root);

                destroy_cnftree(cn_root);
            }
        } else
            fail(ERR_CMDLINE, "Unknown command '%s'\n", action);
    }
}
