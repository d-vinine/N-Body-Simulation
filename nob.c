#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD

#define nob_cc_flags(cmd)                                                      \
  cmd_append(cmd, "-Wall", "-Wextra", "-pedantic", "-std=c23", "-lraylib",     \
             "-lm", "-O3", "-mfma", "-lpthread", "-lrt")

#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  if (!mkdir_if_not_exists(BUILD_FOLDER))
    return 1;

  Cmd cmd = {0};
  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  cmd_append(&cmd, "-o", BUILD_FOLDER "simulation");
  nob_cc_inputs(&cmd, SRC_FOLDER "simulation.c", SRC_FOLDER "sharedmemory.c");

  if (!cmd_run_sync_and_reset(&cmd))
    return 1;

  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  cmd_append(&cmd, "-o", BUILD_FOLDER "renderer");
  nob_cc_inputs(&cmd, SRC_FOLDER "renderer.c", SRC_FOLDER "sharedmemory.c");

  if (!cmd_run_sync_and_reset(&cmd))
    return 1;

  return 0;
}
