#ifndef ROOTHELPER_ROOTHELPER_H_
#define ROOTHELPER_ROOTHELPER_H_

// In the ROOT interpreter, load the installed shared libraries so that only declarations
// have to be parsed. The paths are written at installation. ACLiC sees the same request
// while parsing the macro and links the libraries. Other compiled code links them itself
// (e.g. roothelper::RootHelper in CMake).
#if defined(__CLING__) && !defined(ROOTHELPER_USED_IN_INTERPRETER)
#include <roothelper/cling_libraries.h>
#endif

#include <roothelper/analysis.h>
#include <roothelper/container.h>
#include <roothelper/data_saver.h>
#include <roothelper/graphics.h>

#endif  // ROOTHELPER_ROOTHELPER_H_
