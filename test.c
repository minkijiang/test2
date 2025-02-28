#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
	char word[256];

	FILE* file = fopen("/proc/1/fd/", "r");

	while (fgets(word, 256 ,file) != NULL) {
		printf("%s\n", word);
	}

	return 0;
}