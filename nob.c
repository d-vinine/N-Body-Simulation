#define NOB_IMPLEMENTATION
#include "nob.h"

#define SRC_FOLDER "./src/"
#define BUILD_DIR "./build/"

#define COMPILER "gcc"

void flags(Nob_Cmd *cmd) {
  nob_cmd_append(cmd, "-O3", "-lm", "-lraylib", "-fopenmp");
}

void source(Nob_Cmd *cmd) {
  Nob_File_Paths *children = malloc(sizeof(Nob_File_Paths));
  nob_read_entire_dir(SRC_FOLDER, children);

  nob_da_foreach(const char *, x, children) {
    int idx = strlen(*x) - 1;
    if ((*x)[idx] == 'c') {
      char *file = calloc((strlen(*x) + strlen(SRC_FOLDER)), sizeof(char));
      strcat(file, SRC_FOLDER);
      strcat(file, *x);

      nob_cmd_append(cmd, file);
    }
  }
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  nob_mkdir_if_not_exists(BUILD_DIR);

  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, COMPILER, "-o", BUILD_DIR "main");

  source(&cmd);
  flags(&cmd);

  if (!nob_cmd_run_async(cmd))
    return 1;

  return 0;
}
