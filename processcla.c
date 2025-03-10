#include "processcla.h"

int getThreshold(char* arg) {
	char arg2[MAXLENGTH];
	char threshold[MAXLENGTH];
	strcpy(arg2, arg);

	if (strlen(arg2) > 12) {
		strcpy(threshold, arg+12);
		arg2[12] = '\0';
	}
	else {
		return NOTHING;
	}

	if (strcmp(arg2, "--threshold=") != 0) {
		return NOTHING;
	}

	return strtol(threshold, NULL, 10);
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
