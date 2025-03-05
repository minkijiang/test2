#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#define MAXLENGTH 256
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

void skip(DIR* dir) {
	DIRECTORYINFO directoryInfo;
	directoryInfo = readdir(dir);
	long int pos;
	while (directoryInfo->d_name[0] == '.') {
		pos = telldir(dir);
		directoryInfo = readdir(dir);
	}
	seekdir(dir, pos);

}

char* getProcessDirectory(int pid) {
	char pid_str[MAXLENGTH];
	sprintf(pid_str, "%d", pid);

	char* directoryName = malloc((MAXLENGTH-1)*sizeof(char));
	if (directoryName == NULL) {
		fprintf(stderr, "Error: failed to malloc");
		exit(1);
	}

	strcat(directoryName, "/proc/");
	strcat(directoryName, pid_str);

	return directoryName;

}



int* getStringLengths(PROCESS* process) {
	int maxPidLength = ceil(log10(process->pid));
	int maxFdLength = 0;
	int maxFileLength = 0;
	int maxInodeLength = 0;

	for (int i = 0; i < process->fdCount; i++) {
		int fdLength = 1;
		if (process->FDarr[i]->fd > 0) {fdLength = log10(process->FDarr[i]->fd)+1;}
		int fileLength = strlen(process->FDarr[i]->file);
		int inodeLength = log10(process->FDarr[i]->inode)+1;

		if (fdLength > maxFdLength) {maxFdLength = fdLength;}
		if (fileLength > maxFdLength) {maxFileLength = fileLength;}
		if (inodeLength > maxInodeLength) {maxInodeLength = inodeLength;}
	}

	int* lengths = malloc(4*sizeof(int));
	lengths[0] = maxPidLength;
	lengths[1] = maxFdLength;
	lengths[2] = maxFileLength;
	lengths[3] = maxInodeLength;

	return lengths;
}




FD* createFD(int fd, char file[MAXLENGTH], int inode) {
	FD* newFD = malloc(sizeof(FD));
	if (newFD == NULL) {
		fprintf(stderr, "Error: failed to malloc");
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
		fprintf(stderr, "Error: failed to malloc");
		exit(1);
	}

	newProcess->pid = pid;
	newProcess->processDirectory = getProcessDirectory(pid);
	newProcess->fdCount = 0;
	newProcess->FDarr = NULL;

	return newProcess;
}

DISPLAYINFO* createDISPLAYINFO() {
	DISPLAYINFO* newDisplayInfo = malloc(sizeof(DISPLAYINFO));

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
		free(processes[i]->processDirectory);
		free(processes[i]);
	}
	
}


bool isValidProcess(int pid) {

	if (pid == ALLPID) {
		return true;
	}

	char* processDirectory = getProcessDirectory(pid);
	strcat(processDirectory, "/fd");

	DIR* dir = opendir(processDirectory);
	free(processDirectory);

	if (dir == NULL && errno == ENOENT) {
		int isClosed = closedir(dir);
		if (isClosed != 0) { 
			fprintf(stderr, "Error: failed to close process file");
			exit(1);
		}

		return false;
	}

	int isClosed = closedir(dir);
	if (isClosed != 0) { 
		fprintf(stderr, "Error: failed to close process file");
		exit(1);
	}

	return true;
}

void displayProcessFD(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID");
	int* lengths = getStringLengths(process);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("FD\n");
	for (int i = 0; i < lengths[0]+lengths[1]+strlen("PID")+strlen("FD"); i++) {
		printf("=");
	}
	printf("\n");

	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		printf("%d   %d\n", process->pid, fd);
	}

	for (int i = 0; i < lengths[0]+lengths[1]+strlen("PID")+strlen("FD"); i++) {
		printf("=");
	}
	printf("\n");

	free(lengths);
}

void displaySystemWide(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID");
	int* lengths = getStringLengths(process);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("Filename\n");
	for (int i = 0; i < lengths[0]+lengths[2]+strlen("PID")+strlen("Filename"); i++) {
		printf("=");
	}
	printf("\n");

	for (int i = 0; i < process->fdCount; i++) {
		char* file = process->FDarr[i]->file;
		printf("%d   %s\n", process->pid, file);
	}

	for (int i = 0; i < lengths[0]+lengths[2]+strlen("PID")+strlen("Filename"); i++) {
		printf("=");
	}
	printf("\n");

	free(lengths);
}

void displayVnode(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID");
	int* lengths = getStringLengths(process);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("Inode\n");
	for (int i = 0; i < lengths[0]+lengths[3]+strlen("PID")+strlen("Inode"); i++) {
		printf("=");
	}
	printf("\n");

	for (int i = 0; i < process->fdCount; i++) {
		long long int inode = process->FDarr[i]->inode;
		printf("%d   %lld\n", process->pid, inode);
	}

	for (int i = 0; i < lengths[0]+lengths[3]+strlen("PID")+strlen("Inode"); i++) {
		printf("=");
	}
	printf("\n");

	free(lengths);
}

void displayComposite(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID");
	int* lengths = getStringLengths(process);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("FD");
	for (int i = 0; i < lengths[1]+3-strlen("FD"); i++) {
		printf(" ");
	}
	printf("Filename");
	for (int i = 0; i < lengths[2]+3-strlen("Filename"); i++) {
		printf(" ");
	}
	printf("Inode\n");

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		printf("=");
	}
	printf("\n");

	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		char* file = process->FDarr[i]->file;
		long long int inode = process->FDarr[i]->inode;
		printf("%d   %d   %s   %lld\n", process->pid, fd, file, inode);
	}

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		printf("=");
	}

	printf("\n");

	free(lengths);
}

void writeCompositeTXT(PROCESS* process) {
	FILE* file = fopen("compositeTable.txt", "w");
	if (file == NULL) {
		fprintf(stderr, "Error: could not write to compositeTable.txt");
		exit(1);
	}

	FD** fd = process->FDarr;

	fprintf(file, "\n\n");
	fprintf(file, "PID			FD			Filename			Inode\n");
	fprintf(file, "==================================================\n");
	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		char* filename = process->FDarr[i]->file;
		long long int inode = process->FDarr[i]->inode;
		fprintf(file, "%d   %d   %s   %lld\n", process->pid, fd, filename, inode);
	}
	fprintf(file, "==================================================\n");

	int isClosed = fclose(file);
	if (isClosed != 0) {
		fprintf(stderr, "Error: could not close compositeTable.txt");
		exit(1);
	}
}

void writeCompositeBIN(PROCESS* process) {
	FILE* file = fopen("compositeTable.bin", "wb");

	if (file == NULL) {
		fprintf(stderr, "Error: could not write to compositeTable.bin");
		exit(1);
	}

	FD** fd = process->FDarr;

	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		char* filename = process->FDarr[i]->file;
		long long int inode = process->FDarr[i]->inode;

		fwrite(&process->pid, sizeof(int), 1, file);
		fwrite(&fd, sizeof(int), 1, file);
		fwrite(filename, sizeof(char), MAXLENGTH-1, file); 
		fwrite(&inode, sizeof(long long int), 1, file);
	}
	int end = END;
	fwrite(&end, sizeof(int), 1, file);

	int isClosed = fclose(file);
	if (isClosed != 0) {
		fprintf(stderr, "Error: could not close compositeTable.bin");
		exit(1);
	}
}

void displaySummary(PROCESS** processes, int numProcess) {
	printf("\n\n");
	printf("Summary Table\n");
	printf("=============\n");

	for (int i = 0; i< numProcess; i++) {
		int pid = processes[i]->pid;
		int fdCount = processes[i]->fdCount;
		printf("%d : %d FD\n", pid, fdCount);
	}
	printf("=============\n");
}

void displayOffending(PROCESS** processes, int processNumber, int threshold) {
	printf("\n\n");
	printf("Offending Processes -- Threshold : %d\n", threshold);
	for (int i = 0; i < processNumber; i++) {
		int pid = processes[i]->pid;
		int fdCount = processes[i]->fdCount;
		if (fdCount >= threshold) {
			printf("%d : %d FD\n", pid, fdCount);
		}
		
	}
}

int getFdCount(int pid) {
	char directoryName[MAXLENGTH];

	strcpy(directoryName, getProcessDirectory(pid));
	strcat(directoryName, "/fd");

	DIR* dir = opendir(directoryName);
	if (dir == NULL) {
		fprintf(stderr, "failed to read process directory");
		exit(1);
	}

	skip(dir);
	DIRECTORYINFO directoryInfo = readdir(dir);
	int fdCount = 0;

	while (directoryInfo != NULL) {
		fdCount++;
		directoryInfo = readdir(dir);
	}

	int isClosed = closedir(dir);
	if (isClosed != 0) {
		fprintf(stderr, "failed to close process directory");
		exit(1);
	}

	return fdCount;

}

PROCESS* getProcess(int pid) {
	PROCESS* process = createPROCESS(pid);

	char directoryName[MAXLENGTH];
	strcpy(directoryName, process->processDirectory);
	strcat(directoryName, "/fd");

	DIR* dir = opendir(directoryName);
	if (dir == NULL) {
		fprintf(stderr, "failed to read process directory");
		exit(1);
	}

	process->fdCount = getFdCount(pid);
	process->FDarr = malloc((process->fdCount)*sizeof(FD));

	skip(dir);
	DIRECTORYINFO directoryInfo = readdir(dir);

	for (int i = 0; i < process->fdCount; i++) {

		int fd = strtol(directoryInfo->d_name, NULL, 10);

		char target[MAXLENGTH];

		char link[MAXLENGTH];
		strcpy(link, directoryName);
		strcat(link, "/");
		strcat(link, directoryInfo->d_name);

		if (readlink(link, target, (MAXLENGTH-1)*sizeof(char)) == -1) {
			fprintf(stderr, "failed to read fd");
			exit(1);
		}

		struct stat fileStat;
		if (stat(link, &fileStat) == -1) {
			fprintf(stderr, "failed to read fd");
			exit(1);
		}

		long long int inode = (long long int)fileStat.st_ino;

		process->FDarr[i] = createFD(fd, target, inode);

		directoryInfo = readdir(dir);

	}

	return process;

}

int getAllProcesses(PROCESS** processArr) {
	int processCount = 0;

	DIR* dir = opendir("/proc");
	if (dir == NULL) {
		fprintf(stderr, "failed to read proc directory");
		exit(1);
	}

	skip(dir);
	DIRECTORYINFO directoryInfo = readdir(dir);
	for (int i = 0; directoryInfo != NULL; i++) {
		int pid = strtol(directoryInfo->d_name, NULL, 10);
		if (isValidProcess(pid)) {
			processArr[i] = getProcess(pid);
		}
		directoryInfo = readdir(dir);
		processCount++;
	}

	int isClosed = closedir(dir);
	if (isClosed != 0) {
		fprintf(stderr, "failed to close process directory");
		exit(1);
	}

	return processCount;
}



void display(DISPLAYINFO* displayInfo) {

	PROCESS** allProcesses;
	int processCount = getAllProcesses(allProcesses);

	if (displayInfo->pid == ALLPID) {
		if (displayInfo->isSummary) {displaySummary(allProcesses, processCount);}
		if (displayInfo->threshold != NOTHRESHOLD) {displayOffending(allProcesses, processCount,displayInfo->threshold);}

		for (int i = 0; i < processCount; i++ ) {
			PROCESS* process = *(allProcesses+i);
			if (displayInfo->isProcessFD) {displayProcessFD(process);}
			if (displayInfo->isSystemWide) {displaySystemWide(process);}
			if (displayInfo->isVnode) {displayVnode(process);}
			if (displayInfo->isComposite) {displayComposite(process);}
			if (displayInfo->outputTXT) {writeCompositeTXT(process);}
			if (displayInfo->outputBIN) {writeCompositeBIN(process);}
		}
	}
	else {
		PROCESS* process = getProcess(displayInfo->pid);

		if (displayInfo->isProcessFD) {displayProcessFD(process);}
		if (displayInfo->isSystemWide) {displaySystemWide(process);}
		if (displayInfo->isVnode) {displayVnode(process);}
		if (displayInfo->isComposite) {displayComposite(process);}
		if (displayInfo->isSummary) {displaySummary(allProcesses, processCount);}
		if (displayInfo->threshold != NOTHRESHOLD) {displayOffending(allProcesses, processCount, displayInfo->threshold);}
		if (displayInfo->outputTXT) {writeCompositeTXT(process);}
		if (displayInfo->outputBIN) {writeCompositeBIN(process);}
		freeAllPROCESS(&process, 1);
	}

	freeAllPROCESS(allProcesses, processCount);

	
}


int getThreshold(char* arg) {
	if (strlen(arg) > 12) {
		arg[11] = '\0';
	}
	else {
		return NOTHING;
	}

	if (strcmp(arg, "--threshold") != 0) {
		return NOTHING;
	}

	return strtol(arg+12, NULL, 10);
}

DISPLAYINFO* processArguments(int argc, char** argv) {
	DISPLAYINFO* displayInfo = createDISPLAYINFO();

	if (argc == 1) {
		displayInfo->isComposite = true;
		displayInfo->pid = ALLPID;
	}
	else if (argc == 2 && strtol(argv[1], NULL, 10) > 0) {
		displayInfo->pid = strtol(argv[1], NULL, 10);
		displayInfo->isProcessFD = true;
	    displayInfo->isSystemWide = true;
	    displayInfo->isVnode = true;
	    displayInfo->isComposite = true;
	}
	else {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--per-process") == 0 ) {
				displayInfo->isProcessFD = true;
			}
			else if (strcmp(argv[i], "--systemWide") == 0 ) {
				displayInfo->isSystemWide = true;
			}
			else if (strcmp(argv[i], "--Vnodes") == 0 ) {
				displayInfo->isVnode = true;
			}
			else if (strcmp(argv[i], "--composite") == 0 ) {
				displayInfo->isComposite = true;
			}
			else if (strcmp(argv[i], "--summary") == 0 ) {
				displayInfo->isSummary = true;
			}
			else if (i == 0 && strtol(argv[i], NULL, 10) > 0) {
				displayInfo->pid = strtol(argv[i], NULL, 10);
			}
			else if (getThreshold(argv[i]) != NOTHING) {
				displayInfo->threshold = getThreshold(argv[i]);
			}
			else {
				fprintf(stderr, "Error: invalid arguments");
				exit(1);
			}
		}
	}

	return displayInfo;

}

int test(int argc, char** argv) {

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

void wait_ms(int tdelay) {
	clock_t start_time = clock();
  	while ((clock() - start_time) * 1000 / CLOCKS_PER_SEC < tdelay/1000000);
}


int main() {

	int pid = fork();

	if (pid != 0) {
		PROCESS* process = getProcess(pid);
		displayProcessFD(process);
		displaySystemWide(process);
		displayVnode(process);
		displayComposite(process);
	}
	else {
		wait_ms(2000000);
	}
	


	return 0;
}