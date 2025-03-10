#include "fddisplay.h"

int getDigits(int num) {
	if (num == 0) {
		return 1;
	}
	int digits = log10(num) + 1;
	return digits;
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

bool inStrArray(char** arr, int size, char* word) {
	if (size == 0) {return false;}
	for (int i = 0; i < size; i++) {
		if (strcmp(arr[i], word) == 0) {
			return true;
		}
	}
	return false;
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
		for (int k = 0; process->fdCount; k++) {
			int fd = process->FDarr[k]->fd;

			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);

			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}

			printf("%d\n", fd);
			count++;
		}
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
		for (int k = 0; process->fdCount; k++) {
			long long int inodes = process->FDarr[k]->inode;

			printf("%d", count);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				printf(" ");
			}

			printf("%d", process->pid);
			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				printf(" ");
			}
			printf("%lld\n", inodes);
			count++;
		}
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

void writeCompositeBIN(PROCESS** processes,  int processCount) {
	FILE* file = fopen("compositeTable.bin", "wb");
	if (file == NULL) {
		perror("open compositeTable.bin failed");
		exit(1);
	}

	fwrite("      PID", sizeof(char), 9, file);
	int* lengths = getStringLengths(processes, processCount);
	for (int i = 0; i < lengths[0]+3-strlen("PID"); i++) {
		fwrite(" ", sizeof(char), 1, file);
	}
	fwrite("FD", sizeof(char), 2, file);
	for (int i = 0; i < lengths[1]+3-strlen("FD"); i++) {
		fwrite(" ", sizeof(char), 1, file);
	}
	fwrite("Filename", sizeof(char), 8, file);
	for (int i = 0; i < lengths[2]+3-strlen("Filename"); i++) {
		fwrite(" ", sizeof(char), 1, file);
	}
	fwrite("Inode", sizeof(char), 5, file);

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		fwrite("=", sizeof(char), 1, file);
	}
	fwrite("\n", sizeof(char), 1, file);

	int count = 1;
	for (int i = 0; i < processCount; i++) {
		PROCESS* process = processes[i];
		for (int k = 0; k < process->fdCount; k++) {
			int pid = process->pid;
			int fd = process->FDarr[k]->fd;
			char filename[MAXLENGTH];
			strcpy(filename, process->FDarr[k]->file);
			long long int inode = process->FDarr[k]->inode;

			fwrite(&count, sizeof(int), 1, file);
			for (int j = 0; j < 3+getDigits(lengths[4])-getDigits(count); j++) {
				fwrite(" ", sizeof(char), 1, file);
			}

			fwrite(&pid, sizeof(int), 1, file);
			for (int j = 0; j < lengths[0]+3-getDigits(process->pid); j++) {
				fwrite(" ", sizeof(char), 1, file);
			}

			fwrite(&fd, sizeof(int), 1, file);
			for (int j = 0; j < lengths[1]+3-getDigits(fd); j++) {
				fwrite(" ", sizeof(char), 1, file);
			}
			fwrite(filename, sizeof(char), MAXLENGTH, file);
			for (int j = 0; j < lengths[2]+3-(int)strlen(filename); j++) {
				fwrite(" ", sizeof(char), 1, file);
			}
			fwrite(&inode, sizeof(long long int), 1, file);
			count++;

		}
	}

	for (int i = 0; i < lengths[0]+lengths[1]+lengths[2]+lengths[3]+strlen("PID")+strlen("FD")+strlen("Filename")+strlen("Inode"); i++) {
		fwrite("=", sizeof(char), 1, file);
	}

	free(lengths);

	int isClosed = fclose(file);
	if (isClosed != 0) {
		perror("close compositeTable.txt failed");
		exit(1);
	}
}

/*
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

	int isClosed = fclose(file);
	if (isClosed != 0) {
		perror("close compositeTable.bin failed");
		exit(1);
	}
}
*/

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
