#pragma once

#ifndef bh_sighandler_H
#define bh_sighandler_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#elif (defined(__linux__) || defined(__linux))
#include <unistd.h>
#endif

void signal_handler(int sig);
void initialize_signal_handler();


#endif // bh_sighandler_H