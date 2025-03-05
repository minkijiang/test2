#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <time.h>

#define MAXLENGTH 256
#define NOTHRESHOLD 0
#define ALLPID -1
#define NOTHING -2
#define END -3

typedef struct FD {
	int fd;
	char file[MAXLENGTH];
	int inode;
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
	printf("PID			FD\n");
	printf("==============\n");
	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		printf("%d   %d\n", process->pid, fd);
	}
	printf("==============\n");
}

void displaySystemWide(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID			Filename\n");
	printf("====================\n");
	for (int i = 0; i < process->fdCount; i++) {
		char* file = process->FDarr[i]->file;
		printf("%d   %s\n", process->pid, file);
	}
	printf("====================\n");
}

void displayVnode(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID			Inode\n");
	printf("=================\n");
	for (int i = 0; i < process->fdCount; i++) {
		int inode = process->FDarr[i]->inode;
		printf("%d   %d\n", process->pid, inode);
	}
	printf("=================\n");
}

void displayComposite(PROCESS* process) {
	FD** fd = process->FDarr;

	printf("\n\n");
	printf("PID			FD			Filename			Inode\n");
	printf("=============================================\n");
	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		char* file = process->FDarr[i]->file;
		int inode = process->FDarr[i]->inode;
		printf("%d   %d   %s   %d\n", process->pid, fd, file, inode);
	}
	printf("=============================================\n");
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
		int inode = process->FDarr[i]->inode;
		fprintf(file, "%d   %d   %s   %d\n", process->pid, fd, filename, inode);
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
		int inode = process->FDarr[i]->inode;

		fwrite(&process->pid, sizeof(int), 1, file);
		fwrite(&fd, sizeof(int), 1, file);
		fwrite(filename, sizeof(char), MAXLENGTH-1, file); 
		fwrite(&inode, sizeof(int), 1, file);
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

		struct stat* fileStat;
		if (stat(link, fileStat) == -1) {
			fprintf(stderr, "failed to read fd");
			exit(1);
		}

		int inode = fileStat->st_ino;

		process->FDarr[i] = createFD(fd, target, inode);

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
		if (strtol(directoryInfo->d_name, NULL, 10) > 0) {
			int pid = strtol(directoryInfo->d_name, NULL, 10);
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
		int fd = strtol(directoryInfo->d_name, NULL, 10);

		char target[MAXLENGTH];

		char link[MAXLENGTH];
		strcpy(link, directoryName);
		strcat(link, "/");
		strcat(link, directoryInfo->d_name);


		printf("%ld\n", readlink(link, target, (MAXLENGTH-1)*sizeof(char)) );
		printf("%s\n", target);


		struct stat fileStat;

		int n = stat(link, &fileStat);
		if (n == -1) {
			perror("stat failed");
			return 1;
		}
		else {
			//printf("%lld\n", fileStat.st_ino );
			printf("%d", n);
		}

		//PROCESS* process = getProcess(pid);
		//printf("%d", process->fdCount);
	}
	else {
		wait_ms(2000000);
	}
	


	return 0;
}