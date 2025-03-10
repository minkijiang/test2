#include "getprocess.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


bool isValidProcess(int pid) {

	if (pid == ALLPID) {
		return true;
	}

	char* processDirectory = getProcessDirectoryName(pid);

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


PROCESS* getProcess(int pid) {
	PROCESS* process = createPROCESS(pid);

	DIR* dir = opendir(process->processDirectory);
	if (dir == NULL) {
		perror("open process directory failed");
		exit(1);
	}

	process->FDarr = malloc(sizeof(FD));

	for (DIRECTORYINFO directoryInfo = readdir(dir); directoryInfo != NULL ; directoryInfo = readdir(dir)) {

		if (directoryInfo->d_name[0] != '.') {
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

			FILESTAT fileStat;
			if (stat(link, &fileStat) == -1) {
				perror("stat failed");
				exit(1);
			}

			long long int inode = (long long int)fileStat.st_ino;

			process->FDarr = realloc(process->FDarr, (process->fdCount+1)*sizeof(FD));
			process->FDarr[process->fdCount] = createFD(fd, target, inode);
			process->fdCount += 1;
		}

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

int int main()
{
	
	return 0;
}