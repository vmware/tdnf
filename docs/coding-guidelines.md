# C Coding guidelines for the TDNF Project

Please adhere to the following coding pattern to derive the following benefits.

1. Simplicity
1. Ease of understanding the code
1. Rigorous memory management and hygiene

## Components

The project consists of components each of which may build one of the following.

1. Library (Static or Dynamic)
2. Executable

Associate each library component with a single "public" header file.

A library's public header file must be posted in an "include" folder at a level in the file system at or above its source folder.

### Sources

Each component must have the following file format.

1. One or more "C" files
   1. Define all globals in a file named globals.c
1. Set all definitions in a file named defines.h
1. Define all structures in a file named structs.h
1. Declare globals in a file named externs.h
1. Define all function prototypes that are private to the component in a file named prototypes.h
   1. Arrange the function prototypes sorted in alphabetical order by source file (in which they are incident)
   1. Arrange the function prototypes within each section in the same order as they occur in the corresponding source file
1. Create a single header named includes.h that is the only header included in all the source files. This header may be pre-compiled. The order of headers in this header must be as follows.
   1. config.h (from auto-tools if relevant)
   1. single project level global headers
   1. any system headers that are specific to current component
   1. any headers from component dependencies
   1. defines.h
   1. structs.h
   1. prototypes.h
   1. externs.h

Note:

1. A component must never include header files from within another component's source folder. It may only include header files from the relevant "include" folders.

### Source files

#### Global Variables

1. Try to avoid global variables as much as possible.
2. Do not define or declare global variables in source files.
3. Define the global variables in globals.c and declare them in externs.h

#### Function order

1. Try to order functions in the order they are invoked. For example, functions CreateResource, ReadResource, UpdateResource, DeleteResource will occur in that order in the source file resource.c

#### Static functions

1. Try to avoid static functions as much as possible.
2. Declare static function prototypes at the top of the source file in the same order as they are defined
3. Define the static functions at the bottom of the source file (do not avoid the function declaration at the top)

### Functions

1. If a function has the possibility to fail, it must return a 32 bit error code
2. Functions must have a single exit point
3. Declare all variables are the top of scope
4. Initialize all variables
5. Always allocate memory to local variables and assign to relevant out pointers just before the point of return from the function
6. Use goto in a forward invocation pattern as much as possible. Avoid using goto to create loops.
7. Memory cleanup patterns
   1. Using error label to control single point of exit from function

```
uint32_t
CreateResource(
    resource_t** ppResource,
    const char* pszPath
) {
    uint32_t err = 0;
    FILE* fp = NULL;
    resource_t* pResource = NULL;

    fp = fopen(pszPath, "r);
    if (!fp) {
        err = ERROR_FILE_OPEN;
        goto error;
    }

    err = AllocateResource((void**)&pResource, sizeof(*pResource));
    if (err) {
        goto error;
    }

    *ppResource = pResource;
    pResource = NULL;

error:

    if (pResource) {
        FreeResource(pResource);
    }
    if (fp) {
        fclose(fp);
    }

    return err;
}
```

   1. Using error and cleanup labels to control single point of exit from function. This is easier to understand if there is other memory allocated locally within the function that must be cleaned up locally and never returned out of the function.

```
uint32_t
CreateResource(
    resource_t** ppResource,
    const char* pszPath
) {
    uint32_t err = 0;
    FILE* fp = NULL;
    resource_t* pResource = NULL;

    fp = fopen(pszPath, "r);
    if (!fp) {
        err = ERROR_FILE_OPEN;
        goto error;
    }

    err = AllocateResource((void**)&pResource, sizeof(*pResource));
    if (err) {
        goto error;
    }

    *ppResource = pResource;

cleanup:

    if (fp) {
        fclose(fp);
    }

    return err;

error:

    if (pResource) {
        FreeResource(pResource);
    }

    goto cleanup;
}
```

### Component entry and exit methods

For simplicity, always maintain entry and exit methods per component. These methods must be used to perform any appropriate initialization and cleanup. It may be possible to link these up with init and fini as supported by the compiler.

Dependent components must invoke these entry and exit methods appropriately.

### Memory management

1. Use dedicated methods to allocate and free memory for the component. These will form vantage points to control any advanced memory management features or to debug memory issues.
2. Always initialize the allocated memory (use calloc)

### Error codes

1. Maintain a 32 bit error code range specific to the component

### Documentation

1. Write documentation using Doxygen format
2. Definitely document the header files
3. Write readable code. If the source code is self-explanatory, do not write documentation unless it serves a purpose
