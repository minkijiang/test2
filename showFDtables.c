#include "fdstruct.h"
#include "fddisplay.h"
#include "getprocess.h"
#include "processcla.h"


void display(DISPLAYINFO* displayInfo) {

	int processCount;
	PROCESS** allProcesses = getAllProcesses(&processCount);

	if (displayInfo->pid == ALLPID) {
		if (displayInfo->isSummary) {displaySummary(allProcesses, processCount);}
		if (displayInfo->threshold != NOTHRESHOLD) {displayOffending(allProcesses, processCount,displayInfo->threshold);}

		if (displayInfo->isProcessFD) {displayProcessFD(allProcesses, processCount);}
		if (displayInfo->isSystemWide) {displaySystemWide(allProcesses, processCount);}
		if (displayInfo->isVnode) {displayVnode(allProcesses, processCount);}
		if (displayInfo->isComposite) {displayComposite(allProcesses, processCount);}
		if (displayInfo->outputTXT) {writeCompositeTXT(allProcesses, processCount);}
		if (displayInfo->outputBIN) {writeCompositeBIN(allProcesses, processCount);}
		
	}
	else {
		PROCESS* process = getProcess(displayInfo->pid);

		if (displayInfo->isProcessFD) {displayProcessFD(&process, 1);}
		if (displayInfo->isSystemWide) {displaySystemWide(&process, 1);}
		if (displayInfo->isVnode) {displayVnode(&process, 1);}
		if (displayInfo->isComposite) {displayComposite(&process, 1);}
		if (displayInfo->isSummary) {displaySummary(allProcesses, processCount);}
		if (displayInfo->threshold != NOTHRESHOLD) {displayOffending(allProcesses, processCount, displayInfo->threshold);}
		if (displayInfo->outputTXT) {writeCompositeTXT(&process, 1);}
		if (displayInfo->outputBIN) {writeCompositeBIN(&process, 1);}
		freeAllPROCESS(&process, 1);
	}

	freeAllPROCESS(allProcesses, processCount);
	free(allProcesses);

	
}


/*

void test() {
	int pid = fork();

	int pid2 = 0;
	if (pid != 0) {
		pid2 = fork();
	}

	if (pid != 0 && pid2 != 0) {

		PROCESS** processes = malloc(2*sizeof(PROCESS*));
		processes[0] = getProcess(pid);
		processes[1] = getProcess(pid2);

		displayProcessFD(processes, 2);
		displaySystemWide(processes, 2);
		displayVnode(processes, 2);
		displayComposite(processes, 2);

		writeCompositeTXT(processes, 2);
		writeCompositeBIN(processes, 2);

		int n;
		PROCESS** processes2 = getAllProcesses(&n);

		displaySummary(processes2, n);

		displayOffending(processes2, n, 30);

		freeAllPROCESS(processes, 2);
		freeAllPROCESS(processes2, n);

	}
	else {
		wait_ms(32000000);
	}
}

*/



int main(int argc, char** argv) {
	
	
	DISPLAYINFO* displayInfo = processArguments(argc, argv);

	if (isValidProcess(displayInfo->pid)) {
		display(displayInfo);
	}
	else {
		fprintf(stderr, "Error: invalid processID");
		exit(1);
	}

	free(displayInfo);

	return 0;

}

// cd /cmshome/jiangm95/cscb09w25_space/projects

//.  cd /cmshome/jiangm95/cscb09w25_space/projects/test2

//. gcc -Wall -Werror --std=c99 stuff.c

//.  ssh jiangm95@it-ia3170-03.utsc-labs.utoronto.ca 

// gcc a2.c -lm -fsanitize=address