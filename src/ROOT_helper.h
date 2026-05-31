// Include this header for usage in the ROOT cling interpreter.
// This wrapper is used only for an installed package.
// For interpreter setup, see auxiliary/rootlogon.C.

#ifndef ROOT_HELPER_SRC_INTERPRETER_H_
#define ROOT_HELPER_SRC_INTERPRETER_H_

#define ROOT_HELPER_USED_IN_INTERPRETER

#include "analysis.h"
#include "container.h"
#include "data_saver.h"
#include "graphics.h"
#include "src/analysis.cpp"
#include "src/container.cpp"
#include "src/data_saver.cpp"
#include "src/graphics.cpp"

#endif  // ROOT_HELPER_SRC_INTERPRETER_H_
