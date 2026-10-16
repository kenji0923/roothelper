// Example ~/.rootlogon.C for an installed roothelper (library mode).
//
// Macros only need ROOT_INCLUDE_PATH to contain <prefix>/include and
//   #include <roothelper/roothelper.h>
// which loads the libraries itself. Loading them here as well lets ACLiC (macro.C+) link
// compiled macros against them; it costs little at startup.

void rootlogon() {
  // Uncomment if ROOT_INCLUDE_PATH is not set, adjusting the prefix:
  // gInterpreter->AddIncludePath("/usr/local/include");
  gROOT->ProcessLine("#include <roothelper/cling_libraries.h>");
}
