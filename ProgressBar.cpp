#include "common.h"

ProgressBar::ProgressBar(int steps) {
	this->progress = 0;
	this->steps = steps;
	this->width = 50;
}

ProgressBar::ProgressBar(int width, int steps) {
	this->progress = 0;
	this->width = width;
	this->steps = steps;
}

void ProgressBar::nextStep(FILE* stream) {
	if (stream == NULL) return;
	int hashes1 = width * ((float)progress / (float)steps);
	int hashes2 = width * ((float)(++progress) / (float)steps);
	if (hashes1 == hashes2) return;
	char* bar = (char*)malloc((width + 3) * sizeof(char));
	if (bar == NULL) {
		return;
	}
	bar[0] = '[';
	bar[width + 1] = ']';
	bar[width + 2] = '\0';
	for (int i = 1; i < width + 1; i++) {
		if (i - 1 <= hashes2)
			bar[i] = '#';
		else bar[i] = '_';
	}
	fprintf(stream, "%s\n", bar);
	free(bar);
}