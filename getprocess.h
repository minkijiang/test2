#include "fdstruct.h"

#ifndef GETPROCESS_H
#define GETPROCESS_H

bool isValidProcess(int pid);

PROCESS* getProcess(int pid);

PROCESS** getAllProcesses(int* processCount);

#endif