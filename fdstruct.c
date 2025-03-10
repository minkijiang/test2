#include "fdstruct.h"

char* getProcessDirectoryName(int pid) {
	char pid_str[MAXLENGTH];
	sprintf(pid_str, "%d", pid);

	char* directoryName = malloc(MAXLENGTH*sizeof(char));
	if (directoryName == NULL) {
		perror("malloc failed");
		exit(1);
	}
	directoryName[0] = '\0';
	strcat(directoryName, "/proc/");
	strcat(directoryName, pid_str);
	strcat(directoryName, "/fd");

	return directoryName;

}

FD* createFD(int fd, char file[MAXLENGTH], int inode) {
	FD* newFD = malloc(sizeof(FD));
	if (newFD == NULL) {
		perror("malloc for FD failed");
		exit(1);
	}

	newFD->fd = fd;
	strcpy(newFD->file,file);
	newFD->inode = inode;

	return newFD;
}

PROCESS* createPROCESS(int pid) {
	PROCESS* newProcess = malloc(sizeof(PROCESS));
	if (newProcess == NULL) {
		perror("malloc for PROCESS failed");
		exit(1);
	}

	newProcess->pid = pid;
	newProcess->processDirectory = getProcessDirectoryName(pid);
	newProcess->fdCount = 0;
	newProcess->FDarr = NULL;

	return newProcess;
}

DISPLAYINFO* createDISPLAYINFO() {
	DISPLAYINFO* newDisplayInfo = malloc(sizeof(DISPLAYINFO));

	if (newDisplayInfo == NULL) {
		perror("malloc for DISPLAYINFO failed");
		exit(1);
	}

	newDisplayInfo->isProcessFD = false;
	newDisplayInfo->isSystemWide = false;
	newDisplayInfo->isVnode = false;
	newDisplayInfo->isComposite = false;
	newDisplayInfo->isSummary = false;
	newDisplayInfo->outputTXT = false;
	newDisplayInfo->outputBIN = false;

	newDisplayInfo->pid = ALLPID;
	newDisplayInfo->threshold = NOTHRESHOLD;

	return newDisplayInfo;
}


void freeAllPROCESS(PROCESS** processes, int processCount) {
	for (int i = 0; i < processCount; i++) {

		for (int k = 0; k < processes[i]->fdCount; k++) {
			free(processes[i]->FDarr[k]);
		}
		free(processes[i]->FDarr);
		free(processes[i]->processDirectory);
		free(processes[i]);
	}
	
}