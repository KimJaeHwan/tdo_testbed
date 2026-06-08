#include <stdio.h>
#include <string.h>
#include "dfbench_cases.h"

static void usage(const char *argv0) {
    printf("Usage:\n");
    printf("  %s --list\n", argv0);
    printf("  %s --run-all\n", argv0);
    printf("  %s --run DFB001\n", argv0);
}

int main(int argc, char **argv) {
    size_t count = 0;
    const dfb_case_entry_t *cases = dfb_get_cases(&count);

    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        for (size_t i = 0; i < count; i++) {
            printf("%s %s\n", cases[i].id, cases[i].name);
        }
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--run-all") == 0) {
        for (size_t i = 0; i < count; i++) {
            cases[i].fn();
        }
        printf("OK\n");
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "--run") == 0) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(argv[2], cases[i].id) == 0) {
                cases[i].fn();
                printf("OK %s\n", cases[i].id);
                return 0;
            }
        }
        fprintf(stderr, "Unknown case: %s\n", argv[2]);
        return 2;
    }

    usage(argv[0]);
    return 1;
}
