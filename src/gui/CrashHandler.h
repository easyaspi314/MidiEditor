#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QtGlobal>
#include "../Utils.h"
/**
 * Handles a crash signal.
 * TODO: Windows support.
 */
[[noreturn]] void handler(int sig);


#endif // CRASHHANDLER_H
