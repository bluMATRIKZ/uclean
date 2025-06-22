/*
uclean - Removes temp files from removable media on linux.
Copyright (C) 2025 Connor Thomson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or 
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>

int dry_run = 0;
int forced = 0;
char* base_path = NULL;

const char* trash_targets[] = {
    ".DS_Store",
    "Thumbs.db",
    "__MACOSX",
    ".Spotlight-V100",
    ".fseventsd",
    "System Volume Information",
    ".Trash-",
    "*.tmp",
    "*.~",
    NULL
};

int match_target(const char* name) {
    for (int i = 0; trash_targets[i]; i++) {
        if (fnmatch(trash_targets[i], name, 0) == 0) return 1;
        if (strncmp(trash_targets[i], name, strlen(trash_targets[i])) == 0) return 1;
    }
    return 0;
}

int remove_entry(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    const char* name = fpath + ftwbuf->base;

    if (match_target(name)) {
        if (dry_run) {
            printf("[DRY] Would remove: %s\n", fpath);
        } else {
            int res = (typeflag == FTW_D || typeflag == FTW_DP) ? rmdir(fpath) : remove(fpath);
            printf("%s %s\n", (res == 0 ? "[OK]" : "[ERR]"), fpath);
        }
    }

    return 0;
}

int starts_with(const char* path, const char* prefix) {
    return strncmp(path, prefix, strlen(prefix)) == 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mount path> [-d|--dry-run] [-f|--force]\n", argv[0]);
        return 1;
    }

    base_path = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--dry-run") == 0 || strcmp(argv[i], "-d") == 0) {
            dry_run = 1;
    }

        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) forced = 1;
    }

    if (getuid() == 0 && !forced) {
        fprintf(stderr, "Refusing to run as root. Use --force if you're sure.\n");
        return 1;
    }

    if (!starts_with(base_path, "/media/") && !starts_with(base_path, "/run/media/") && !forced) {
        fprintf(stderr, "Not a removable mount point. Use --force to override.\n");
        return 1;
    }

    printf("Cleaning up: %s %s\n", base_path, (dry_run ? "(dry run)" : ""));
    nftw(base_path, remove_entry, 10, FTW_DEPTH | FTW_PHYS);

    return 0;
}
