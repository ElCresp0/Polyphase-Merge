#pragma once
class ProgressBar
{
private:
	int width;
	int steps;
	int progress;
public:
	ProgressBar(int steps);
	ProgressBar(int width, int steps);

	// increments the progress, prints the progress bar to stream if it changes
	void nextStep(FILE* stream);
};

