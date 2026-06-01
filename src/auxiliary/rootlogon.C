#include <TInterpreter.h>
#include <TSystem.h>


/**
 * Add the content of rootlogon() to your function in rootlogon.C.
 */
void rootlogon()
{
    /**
     * Modify this to the installed path of roothelper
     */
    const char* roothelper_include_path = "-I/usr/local/include/roothelper";

    gInterpreter->AddIncludePath(roothelper_include_path);
    gSystem->AddIncludePath(roothelper_include_path);
}
