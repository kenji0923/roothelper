// Source mode for the ROOT cling interpreter: compiles the roothelper sources in the
// interpreter instead of loading the installed libraries. Slower to include (about 1.6 s
// with ROOT 6.36) but needs no shared libraries. Installed as
// <roothelper/roothelper_source.h>; prefer <roothelper/roothelper.h> (library mode).

#ifndef ROOTHELPER_SRC_INTERPRETER_H_
#define ROOTHELPER_SRC_INTERPRETER_H_

#define ROOTHELPER_USED_IN_INTERPRETER

// Order matters: container.h uses graphics.h without including it in this mode.
// clang-format off
#include "analysis.h"
#include "graphics.h"
#include "container.h"
#include "data_saver.h"
#include "src/analysis.cpp"
#include "src/graphics.cpp"
#include "src/container.cpp"
#include "src/data_saver.cpp"
// clang-format on

#endif  // ROOTHELPER_SRC_INTERPRETER_H_
