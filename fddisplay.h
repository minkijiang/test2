#include "fdstruct.h"

#ifndef FDDISPLAY_H
#define FDDISPLAY_H

void displayProcessFD(PROCESS** processes, int processCount);

void displaySystemWide(PROCESS** processes, int processCount);

void displayVnode(PROCESS** processes, int processCount);

void displayComposite(PROCESS** processes, int processCount);

void writeCompositeTXT(PROCESS** processes,  int processCount);

void writeCompositeBIN(PROCESS** processes, int processCount);

void displaySummary(PROCESS** processes, int numProcess);

void displayOffending(PROCESS** processes, int processNumber, int threshold);

#endif

