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

int getDigits(int num) {
	if (num == 0) {
		return 1;
	}
	int digits = log10(num) + 1;
	return digits;
}

char* getProcessDirectory(int pid) {
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



int* getStringLengths(PROCESS** processes, int processCount) {
	int maxPidLength = 0;
	int maxFdLength = 0;
	int maxFileLength = 0;
	int maxInodeLength = 0;

	int totalFdCount = 0;

	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		for (int k = 0; k < process->fdCount; k++) {
			int pidLength = getDigits(process->pid);
			int fdLength = getDigits(process->FDarr[k]->fd); 
			int fileLength = strlen(process->FDarr[k]->file);
			int inodeLength = getDigits(process->FDarr[k]->inode);

			if (pidLength > maxPidLength) {maxPidLength = pidLength;}
			if (fdLength > maxFdLength) {maxFdLength = fdLength;}
			if (fileLength > maxFileLength) {maxFileLength = fileLength;}
			if (inodeLength > maxInodeLength) {maxInodeLength = inodeLength;}

			totalFdCount++;
		}

		
	}

	int* lengths = malloc(5*sizeof(int));
	if (lengths == NULL) {
		perror("malloc failed: ");
		exit(1);
	}

	lengths[0] = maxPidLength;
	lengths[1] = maxFdLength;
	lengths[2] = maxFileLength;
	lengths[3] = maxInodeLength;
	lengths[4] = totalFdCount;

	return lengths;
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
	newProcess->processDirectory = getProcessDirectory(pid);
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


bool isValidProcess(int pid) {

	if (pid == ALLPID) {
		return true;
	}

	char* processDirectory = getProcessDirectory(pid);

	DIR* dir = opendir(processDirectory);
	free(processDirectory);

	if (dir == NULL && (errno == ENOENT || errno == EACCES)) {return false;}
	else if (dir == NULL) {
		perror("open process directory failed");
		exit(1);
	}

	int isClosed = closedir(dir);
	if (isClosed != 0) { 
		perror("close process directory failed");
		exit(1);
	}

	return true;
}

bool inIntArray(int* arr, int size, int num) {
	if (size == 0) {return false;}
	for (int i = 0; i < size; i++) {
		if (arr[i] == num) {
			return true;
		}
	}
	return false;
}

bool inLLDArray(long long int* arr, int size, int num) {
	if (size == 0) {return false;}
	for (int i = 0; i < size; i++) {
		if (arr[i] == num) {
			return true;
		}
	}
	return false;
}

bool inStrArray(char** arr, int size, char* word) {
	if (size == 0) {return false;}
	for (int i = 0; i < size; i++) {
		if (strcmp(arr[i], word) == 0) {
			return true;
		}
	}
	return false;
}

int* getDistinctFDs(PROCESS* process) {
	int* arr = malloc(sizeof(int));

	if (arr == NULL) {
		perror("malloc failed");
		exit(1);
	}

	int count = 0;
	for (int i = 0; i < process->fdCount; i++) {
		int fd = process->FDarr[i]->fd;
		if (!inIntArray(arr, count, fd)) {
			arr = realloc(arr, (count+1)*sizeof(int));
			arr[count] = fd;
			count++;
		}
	}
	arr = realloc(arr, (count+1)*sizeof(int));
	arr[count] = END;

	return arr;
}

char** getDistinctFiles(PROCESS* process) {
	char** arr = malloc(sizeof(char*));

	if (arr == NULL) {
		perror("malloc failed");
		exit(1);
	}

	int count = 0;
	for (int i = 0; i < process->fdCount; i++) {
		char file[MAXLENGTH];
		strcpy(file, process->FDarr[i]->file);
		if (!inStrArray(arr, count, file)) {
			arr = realloc(arr, (count+1)*sizeof(char*));
			arr[count] = malloc(MAXLENGTH*sizeof(char));
			strcpy(arr[count], file);
			count++;
		}
	}

	arr = realloc(arr, (count+1)*sizeof(char*));
	arr[count] = NULL;

	return arr;
}

long long int* getDistinctInodes(PROCESS* process) {
	long long int* arr = malloc(sizeof(long long int));

	if (arr == NULL) {
		perror("malloc failed");
		exit(1);
	}

	int count = 0;
	for (int i = 0; i < process->fdCount; i++) {
		int inode = process->FDarr[i]->inode;
		if (!inLLDArray(arr, count, inode)) {
			arr = realloc(arr, (count+1)*sizeof(long long int));
			arr[count] = inode;
			count++;
		}
	}
	arr = realloc(arr, (count+1)*sizeof(long long int));
	arr[count] = END;

	return arr;
}

void displayProcessFD(PROCESS** processes, int processCount) {
	
	printf("\n\n");
	printf("      PID");
	int* lengths = getStringLengths(processes, processCount);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("FD\n");
	for (int i = 0; i < lengths[0]+lengths[1]+strlen("PID")+strlen("FD"); i++) {
		printf("=");
	}
	printf("\n");

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		int* fds = getDistinctFDs(process);
		for (int k = 0; fds[k] != END; k++) {
			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);

			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}

			printf("%d\n", fds[k]);
			count++;
		}
		free(fds);
	}

	for (int i = 0; i < lengths[0]+lengths[1]+strlen("PID")+strlen("FD"); i++) {
		printf("=");
	}
	printf("\n");

	free(lengths);

}

void displaySystemWide(PROCESS** processes, int processCount) {

	printf("\n\n");
	printf("      PID");
	int* lengths = getStringLengths(processes, processCount);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("Filename\n");
	for (int i = 0; i < lengths[0]+lengths[2]+strlen("PID")+strlen("Filename"); i++) {
		printf("=");
	}
	printf("\n");

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		char** files = getDistinctFiles(process);
		for (int k = 0; files[k] != NULL; k++) {
			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);

			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}

			printf("%s\n", files[k]);
			count++;

		}

		for (int k = 0; files[k] != NULL; k++) {
			free(files[k]);
		}
		free(files);
	}

	for (int i = 0; i < lengths[0]+lengths[2]+strlen("PID")+strlen("Filename"); i++) {
		printf("=");
	}

	printf("\n");

	free(lengths);
}

void displayVnode(PROCESS** processes, int processCount) {

	printf("\n\n");
	printf("      PID");
	int* lengths = getStringLengths(processes, processCount);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		printf(" ");
	}
	printf("Inode\n");
	for (int i = 0; i < lengths[0]+lengths[3]+strlen("PID")+strlen("Inode"); i++) {
		printf("=");
	}
	printf("\n");

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		long long int* inodes = getDistinctInodes(process);
		for (int k = 0; inodes[k] != END; k++) {
			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);
			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}
			printf("%lld\n", inodes[k]);
			count++;
		}
		free(inodes);
	}

	for (int i = 0; i < lengths[0]+lengths[3]+strlen("PID")+strlen("Inode"); i++) {
		printf("=");
	}
	printf("\n");

	free(lengths);
}

void displayComposite(PROCESS** processes, int processCount) {

	printf("\n\n");
	printf("      PID");
	int* lengths = getStringLengths(processes, processCount);
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

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		for (int k = 0; k < process->fdCount; k++) {
			int fd = process->FDarr[k]->fd;
			char filename[MAXLENGTH];
			strcpy(filename, process->FDarr[k]->file);
			long long int inode = process->FDarr[k]->inode;
			
			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);
			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}

			
			printf("%d", fd);
			for (int j = 0; j < lengths[1]+3-getDigits(fd); j++) {
				printf(" ");
			}

			
			printf("%s", filename);

			
			for (long int j = 0; j < lengths[2]+3-(int)strlen(filename); j++) {
				printf(" ");
			}
			
			printf("%lld\n", inode);
			count++;
			

		}
	}

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		printf("=");
	}


	printf("\n");

	free(lengths);
}

void writeCompositeTXT(PROCESS** processes,  int processCount) {
	FILE* file = fopen("compositeTable.txt", "w");
	if (file == NULL) {
		perror("open compositeTable.txt failed");
		exit(1);
	}

	fprintf(file, "      PID");
	int* lengths = getStringLengths(processes, processCount);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		fprintf(file, " ");
	}
	fprintf(file, "FD");
	for (int i = 0; i < lengths[1]+3-strlen("FD"); i++) {
		fprintf(file, " ");
	}
	fprintf(file, "Filename");
	for (int i = 0; i < lengths[2]+3-strlen("Filename"); i++) {
		fprintf(file, " ");
	}
	fprintf(file, "Inode\n");

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		fprintf(file, "=");
	}
	fprintf(file, "\n");

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		for (int k = 0; k < process->fdCount; k++) {
			int fd = process->FDarr[k]->fd;
			char filename[MAXLENGTH];
			strcpy(filename, process->FDarr[k]->file);
			long long int inode = process->FDarr[k]->inode;

			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			fprintf(file, "%d", process->pid);
			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				fprintf(file, " ");
			}

			fprintf(file, "%d", fd);
			for (int j = 0; j < lengths[1]+3-getDigits(fd); j++) {
				fprintf(file, " ");
			}
			fprintf(file, "%s", filename);
			for (int j = 0; j < lengths[2]+3-(int)strlen(filename); j++) {
				fprintf(file, " ");
			}
			fprintf(file, "%lld\n", inode);
			count++;

		}
	}

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		fprintf(file, "=");
	}

	free(lengths);

	int isClosed = fclose(file);
	if (isClosed != 0) {
		perror("close compositeTable.txt failed");
		exit(1);
	}
}

void writeCompositeBIN(PROCESS** processes, int processCount) {
	FILE* file = fopen("compositeTable.bin", "wb");

	if (file == NULL) {
		perror("open compositeTable.bin failed");
		exit(1);
	}

	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		for (int k = 0; k < process->fdCount; k++) {
			int fd = process->FDarr[k]->fd;
			char filename[MAXLENGTH];
			strcpy(filename, process->FDarr[k]->file);
			long long int inode = process->FDarr[k]->inode;

			fwrite(&process->pid, sizeof(int), 1, file);
			fwrite(&fd, sizeof(int), 1, file);
			fwrite(filename, sizeof(char), MAXLENGTH-1, file); 
			fwrite(&inode, sizeof(long long int), 1, file);
		}
	}

	int end = END;
	fwrite(&end, sizeof(int), 1, file);

	int isClosed = fclose(file);
	if (isClosed != 0) {
		perror("close compositeTable.bin failed");
		exit(1);
	}
}

void displaySummary(PROCESS** processes, int numProcess) {
	printf("\n\n");
	printf("Summary Table\n");
	printf("==================\n");

	for (int i = 0; i < numProcess; i++) {
		int pid = processes[i]->pid;
		int fdCount = processes[i]->fdCount;
		printf("%d : %d FDs\n", pid, fdCount);
	}
	printf("==================\n");
}

void displayOffending(PROCESS** processes, int processNumber, int threshold) {
	printf("\n\n");
	printf("Offending Processes -- Threshold : %d\n", threshold);
	for (int i = 0; i < processNumber; i++) {
		int pid = processes[i]->pid;
		int fdCount = processes[i]->fdCount;
		if (fdCount >= threshold) {
			printf("%d : %d FDs\n", pid, fdCount);
		}
		
	}
}

PROCESS* getProcess(int pid) {
	PROCESS* process = createPROCESS(pid);

	DIR* dir = opendir(process->processDirectory);
	if (dir == NULL) {
		perror("open process directory failed");
		exit(1);
	}

	process->FDarr = malloc(sizeof(FD));

	skip(dir);

	for (DIRECTORYINFO directoryInfo = readdir(dir); directoryInfo != NULL ; directoryInfo = readdir(dir)) {

		int fd = strtol(directoryInfo->d_name, NULL, 10);

		char target[MAXLENGTH];

		char link[MAXLENGTH];
		strcpy(link, process->processDirectory);
		strcat(link, "/");
		strcat(link, directoryInfo->d_name);

		int targetLength = readlink(link, target, (MAXLENGTH-1)*sizeof(char));
		if (targetLength == -1) {
			perror("readlink failed");
			exit(1);
		}
		target[targetLength] = '\0';

		struct stat fileStat;
		if (stat(link, &fileStat) == -1) {
			perror("stat failed");
			exit(1);
		}

		long long int inode = (long long int)fileStat.st_ino;

		process->FDarr = realloc(process->FDarr, (process->fdCount+1)*sizeof(FD));
		process->FDarr[process->fdCount] = createFD(fd, target, inode);
		process->fdCount += 1;

	}

	int isClosed = closedir(dir);
	if (isClosed != 0) {
		perror("close process directory failed");
		exit(1);
	}

	return process;


}


PROCESS** getAllProcesses(int* processCount) {

	PROCESS** processes = malloc(sizeof(PROCESS*));

	DIR* dir = opendir("/proc");
	if (dir == NULL) {
		perror("open proc directory failed");
		exit(1);
	}

	skip(dir);
	*processCount = 0;

	for (DIRECTORYINFO directoryInfo = readdir(dir) ; directoryInfo != NULL; directoryInfo = readdir(dir)) {
		int pid = strtol(directoryInfo->d_name, NULL, 10);
		if (isValidProcess(pid)) {
			processes = realloc(processes, ((*processCount)+1)*sizeof(PROCESS*));
			processes[*processCount] = getProcess(pid);
			(*processCount)++;
		}
	}

	int isClosed = closedir(dir);
	if (isClosed != 0) {
		perror("close proc directory failed");
		exit(1);
	}

	return processes;
}



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


int getThreshold(char* arg) {
	char arg2[MAXLENGTH];
	strcpy(arg2, arg);

	if (strlen(arg2) > 12) {
		arg2[11] = '\0';
	}
	else {
		return NOTHING;
	}

	if (strcmp(arg2, "--threshold") != 0) {
		return NOTHING;
	}

	return strtol(arg2+12, NULL, 10);
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
			else if (i == 1 && strtol(argv[i], NULL, 10) > 0) {
				displayInfo->pid = strtol(argv[i], NULL, 10);
			}
			else if (getThreshold(argv[i]) != NOTHING) {
				displayInfo->threshold = getThreshold(argv[i]);
			}
			else if (strcmp(argv[i], "--output_TXT") == 0) {
				displayInfo->outputTXT = true;
			}
			else if (strcmp(argv[i], "--output_binary") == 0) {
				displayInfo->outputBIN = true;
			}
			else {
				fprintf(stderr, "Error: invalid arguments");
				exit(1);
			}
			
		}
	}

	return displayInfo;

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