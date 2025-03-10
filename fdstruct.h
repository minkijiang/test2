#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>

#ifndef STRUCT_H
#define STRUCT_H

#define MAXLENGTH 1024
#define NOTHRESHOLD 0
#define ALLPID -1
#define NOTHING -2
#define END -3

typedef struct FD {
	int fd;
	char file[MAXLENGTH];
	long long int inode;
} FD;

typedef struct PROCESS {
	int fdCount;
	int pid;
	char* processDirectory;
	FD** FDarr;
} PROCESS;

typedef struct DISPLAYINFO {
	bool isProcessFD;
	bool isSystemWide;
	bool isVnode;
	bool isComposite;
	bool isSummary;
	bool outputTXT;
	bool outputBIN;

	int pid;
	int threshold;
} DISPLAYINFO;

typedef struct dirent* DIRECTORYINFO;

typedef struct stat FILESTAT;

char* getProcessDirectoryName(int pid);

FD* createFD(int fd, char file[MAXLENGTH], int inode);

PROCESS* createPROCESS(int pid);

DISPLAYINFO* createDISPLAYINFO();

void freeAllPROCESS(PROCESS** processes, int processCount);

#endif