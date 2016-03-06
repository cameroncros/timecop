#include "main.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <ncurses.h>
#include <gtest/gtest.h>
#include <pwd.h>

char* file = NULL;
bool interactive = false;
bool slimOutput = false;

void printHelp(char *str, int *totalCount, line** allLines) {
	if (interactive) {
		clear();
		move(0, 0);
		printw("%s [options]\n\n", str);
		printw("-a {name}\tAdd a new task\n");
		printw("-d {id}\t\tDelete a task\n");
		printw("-c\t\tdelete(clear) all timers\n");
		printw("-r\t\treset all timers\n");
		printw("[+/-]###[H,M,S] {id} \tIncrement/Decrement timer\n");

		printw("{id}\t\tSwitch to given task\n\n");
		printw("Press any key to continue");
		while (getch() == ERR) {
			usleep(1);
		}

	} else {
		printf("%s [options]\n\n", str);
		printf("-a {name}\tAdd a new task\n");
		printf("-d {id}\t\tDelete a task\n");
		printf("-c\t\tdelete(clear) all timers\n");
		printf("-r\t\treset all timers\n");
		printf("-i\t\tinteractive mode");
		printf("-s\t\tslim output mode, useful for .bashrc");
		printf("[+/-]###[H,M,S] {id} \tIncrement/Decrement timer\n");

		printf("{id}\t\tSwitch to given task\n\n");
		cleanup(totalCount, allLines);
	}
}

line* processLine(char *str) {
	if (str == NULL) {
		return NULL;
	}
	line *parsed = (line *)calloc(1, sizeof(line));

	char *space = strchr(str, ' ');

	if (space == NULL) {
		//invalid format, but handled
		parsed->name = (char *)calloc(1, sizeof(char) * strlen(str));
		strcpy(parsed->name, str);
		return parsed;
	}
	int len = space - str;
	parsed->name = (char *)calloc(1, sizeof(char) * len);
	strncpy(parsed->name, str, len);

	char *nextVal;
	parsed->timeSpent = strtol(space + 1, &nextVal, 10);
	if (*nextVal != '\0' && *nextVal != '\n' && *nextVal != ' ') {
		return NULL;
	}
	if (*nextVal != '\0' && *nextVal != '\n') {
		char *nextChar;
		parsed->timeStarted = strtol(nextVal + 1, &nextChar, 10);
		if (*nextChar != '\0' && *nextChar != '\n') {
			return NULL;
		}
	}
	return parsed;
}

void readFile(int *totalCount, line** allLines) {
	(*totalCount) = 0;
	FILE* fp = fopen(file, "r");
	if (fp == NULL) {
		return;
	}
	ssize_t read;
	size_t len = 0;
	char* line = (char *)calloc(100, sizeof(char));

	while ((read = getline(&line, &len, fp)) != -1) {
		allLines[*totalCount] = processLine(line);
		if (allLines[*totalCount] == NULL) {
			printf("Invalid Format");
			cleanup(totalCount, allLines);
			exit(1);
		}
		(*totalCount)++;

	}
	free(line);
	fclose(fp);
}

void cleanup(int *totalCount, line** allLines) {
	for (int i = *totalCount; i > 0; i--) {
		deleteTaskIndex(totalCount, i - 1, allLines);
	}
	free(allLines);
	free(file);

	if (interactive) {
		endwin();
	}
	exit(0);

}

void writeFile(int totalCount, line** allLines) {
	FILE* fp = fopen(file, "w");

	for (int i = 0; i < totalCount; i++) {
		fprintf(fp, "%s %li", allLines[i]->name, allLines[i]->timeSpent);
		if (allLines[i]->timeStarted != 0) {
			fprintf(fp, " %li", allLines[i]->timeStarted);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

void printTimeSheet(int totalCount, line** allLines) {
	long totalTimeSpent = 0;
	for (int i = 0; i < totalCount; i++) {
		long timerTimeSpent = allLines[i]->timeSpent;
		if (allLines[i]->timeStarted != 0) {
			timerTimeSpent += (time(NULL) - allLines[i]->timeStarted);
		}
		totalTimeSpent += timerTimeSpent;

		int hours = timerTimeSpent / 3600;
		int minutes = (timerTimeSpent - hours * 3600) / 60;
		int seconds = timerTimeSpent - hours * 3600 - minutes * 60;
		if (interactive) {
			move(i, 0);
			printw("%i: ", i + 1);
			if (allLines[i]->timeStarted != 0) {
				printw("* ");
			} else {
				printw("  ");
			}

			printw("%s\t\t\t %02i:%02i:%02i\n", allLines[i]->name, hours,
					minutes, seconds);
		} else if (!slimOutput) {
			printf("%i: ", i + 1);
			if (allLines[i]->timeStarted != 0) {
				printf("* ");
			} else {
				printf("  ");
			}
			printf("%s\t\t\t %02i:%02i:%02i\n", allLines[i]->name, hours,
					minutes, seconds);
		}
	}
	int hours = totalTimeSpent / 3600;
	int minutes = (totalTimeSpent - hours * 3600) / 60;
	int seconds = totalTimeSpent - hours * 3600 - minutes * 60;

	if (interactive) {
		move(totalCount + 1, 0);
		printw("Total: \t\t\t\t %02i:%02i:%02i\n", hours, minutes, seconds);
	} else if (!slimOutput) {
		printf("Total: \t\t\t\t %02i:%02i:%02i\n", hours, minutes, seconds);
	} else {
		printf("%02i:%02i:%02i", hours, minutes, seconds);
	}

	bool negative = false;
	int timeRemaining = 7.6 * 60 * 60 - totalTimeSpent;
	if (timeRemaining < 0) {
		timeRemaining = abs(timeRemaining);
		negative = true;

	}
	hours = timeRemaining / 3600;
	minutes = (timeRemaining - hours * 3600) / 60;
	seconds = timeRemaining - hours * 3600 - minutes * 60;
	if (interactive) {
		move(totalCount + 2, 0);
		printw("Total Remaining:\t\t");
		if (negative) {
			printw("-");
		} else {
			printw(" ");
		}
		printw("%02i:%02i:%02i\n", hours, minutes, seconds);
	} else if (!slimOutput){
		printf("Total Remaining:\t\t");
		if (negative) {
			printf("-");
		} else {
			printf(" ");
		}
		printf("%02i:%02i:%02i\n", hours, minutes, seconds);
	}

	time_t timeEnd = time(NULL) + 7.6 * 60 * 60 - totalTimeSpent;
	struct tm *timeinfo;
	timeinfo = localtime(&timeEnd);
	timeEnd = timeEnd % (24 * 60 * 60);
	hours = timeinfo->tm_hour;
	minutes = timeinfo->tm_min;
	seconds = timeinfo->tm_sec;
	if (interactive) {
		move(totalCount + 3, 0);
		printw("Time End: \t\t\t %02i:%02i:%02i\n", hours, minutes, seconds);
	} else if (!slimOutput){
		printf("Time End: \t\t\t %02i:%02i:%02i\n", hours, minutes, seconds);
	}
}

void deleteTimers(int *totalCount) {
	(*totalCount) = 0;
}

void resetTimers(int totalCount, line** allLines) {
	for (int i = 0; i < totalCount; i++) {
		allLines[i]->timeSpent = 0;
		allLines[i]->timeStarted = 0;
	}
}

void addTask(int *totalCount, char* name, line** allLines) {
	line* parsed = (line *)calloc(1, sizeof(line));
	int nameLen = strlen(name);
	for (int i = 0; i < nameLen; i++) {
		if ((name[i] <= 'z' && name[i] >= 'a')
				|| (name[i] <= 'Z' && name[i] >= 'A')
				|| (name[i] <= '9' && name[i] >= '0')) {
			continue;
		}
		name[i] = '_';
	}
	parsed->name = (char *)malloc(sizeof(char) * nameLen);
	strcpy(parsed->name, name);
	allLines[*totalCount] = parsed;
	(*totalCount)++;
}

void deleteTaskIndex(int *totalCount, int index, line** allLines) {
	free(allLines[index]->name);
	free(allLines[index]);
	for (int i = index; i < *totalCount - 1; i++) {
		allLines[i] = allLines[i + 1];
	}
	(*totalCount)--;
}

int getValidIndex(const char *id, int totalCount) {
	char *nextChar;
	int index = strtol(id, &nextChar, 10);
	if (*nextChar != '\0' || index <= 0
			|| index > totalCount) {
		return -1;
	}
	return index-1;
}

void deleteTask(int *totalCount, char* id, line** allLines) {

	int deleteIndex = getValidIndex(id, *totalCount);
	if (deleteIndex == -1) {
		printf("Invalid Task ID\n");
		return;
	}
	deleteTaskIndex(totalCount, deleteIndex, allLines);
}

void startTimer(int totalCount, char** argv, line** allLines) {
	long selected = getValidIndex(argv[1], totalCount);
	if (selected == -1) {
		printf("Invalid Task ID\n");
		printHelp(argv[0], &totalCount, allLines);
		return;
	}
	for (int i = 0; i < totalCount; i++) {
		if (i == selected) {
			if (allLines[i]->timeStarted == 0) {
				allLines[i]->timeStarted = time(NULL);
			}
		} else if (allLines[i]->timeStarted != 0) {
			long currentTime = time(NULL);
			allLines[i]->timeSpent += (currentTime - allLines[i]->timeStarted);
			allLines[i]->timeStarted = 0;
		}
	}
}

void pauseTimers(int totalCount, line** allLines) {
	for (int i = 0; i < totalCount; i++) {
		if (allLines[i]->timeStarted != 0) {
			long currentTime = time(NULL);
			allLines[i]->timeSpent += (currentTime - allLines[i]->timeStarted);
			allLines[i]->timeStarted = 0;
		}
	}
}

void incrementTime(int totalCount, line** allLines, char*incAmount, char *id) {
	char *lastChar;
	long timeAdd = strtol(incAmount + 1, &lastChar, 10);
	switch (*lastChar) {
	case 'M':
	case 'm':
		timeAdd *= 60;
		break;
	case 'H':
	case 'h':
		timeAdd *= 3600;
		break;
	case 'S':
	case 's':
		timeAdd *= 1;
		break;
	default:
		printf("Invalid Format\n");
		return;
	}

	long selected = getValidIndex(lastChar+2, totalCount);
	if (selected == -1) {
		printf("Invalid Task ID\n");
		return;
	}

	if (incAmount[0] == '+') {
		allLines[selected]->timeSpent += timeAdd;
	} else {
		allLines[selected]->timeSpent -= timeAdd;
	}
}

int readStandardInput(char** argv) {
	argv[2] = NULL;
	if (strlen(argv[1]) != 0) {
		argv[2] = strchr(argv[1], ' ');
		if (argv[2] != NULL && strlen(argv[2]) != 0) {
			argv[2][0] = '\0';
			argv[2]++;
			if (strchr(argv[2], ' ') != NULL) {
				return 4;
			}
			return 3;
		}
		return 2;
	}
	return 1;
}

void interactiveConsole(int *totalCount, line** allLines) {
	interactive = true;
	initscr();
	cbreak();
	echo();
	timeout(1);

	int argc = 0;
	char** argv = (char **)calloc(3, sizeof(char*));
	argv[0] = (char *)calloc(100, sizeof(char));
	argv[1] = (char *)calloc(100, sizeof(char));

	char *str = argv[1];
	strcpy(argv[0], "timecop");

	int ptr = 0;
	int chr = 0;

	while (true) {
		bool typingStarted = false;
		if ((chr = getch()) != ERR) {

			typingStarted = true;
			switch (chr) {
			case 10:
			case KEY_ENTER:
				argc = readStandardInput(argv);
				parseCommand(argc, argv, totalCount, allLines);
				ptr = 0;
				str[ptr] = '\0';
				typingStarted = false;
				break;
			case 127:
			case KEY_BACKSPACE:
				if (ptr != 0) {
					str[ptr--] = '\0';
				}
				break;
			default:
				str[ptr++] = (char) chr;
				str[ptr + 1] = '\0';
				if (ptr == 100) {
					ptr = 0;
				}
				break;
			}
		}
		clear();
		printTimeSheet(*totalCount, allLines);
		move(*totalCount + 5, 0);
		printw("> %s", str);
		move(*totalCount + 5, ptr + 2);
		refresh();
		if (!typingStarted) {
			sleep(1);
		}
	}
}

void parseCommand(int argc, char** argv, int *totalCount, line** allLines) {
	if (argc == 2) {
		if (strcmp(argv[1], "-r") == 0) {
			resetTimers(*totalCount, allLines);
			writeFile(*totalCount, allLines);
		} else if (strcmp(argv[1], "-c") == 0) {
			deleteTimers(totalCount);
			writeFile(*totalCount, allLines);
		} else if (strcmp(argv[1], "-p") == 0) {
			pauseTimers(*totalCount, allLines);
			writeFile(*totalCount, allLines);
		} else if (strcmp(argv[1], "-h") == 0) {
			printHelp(argv[0], totalCount, allLines);
		} else if (strcmp(argv[1], "-q") == 0) {
			cleanup(totalCount, allLines);
		} else if (strcmp(argv[1], "-i") == 0 && interactive != true) {
			interactiveConsole(totalCount, allLines);
		} else if (strcmp(argv[1], "-s") == 0 && interactive != true) {
			slimOutput = true;
		} else {
			startTimer(*totalCount, argv, allLines);
			writeFile(*totalCount, allLines);
		}
	} else if (argc == 3) {
		if (strcmp(argv[1], "-a") == 0) {
			addTask(totalCount, argv[2], allLines);
			writeFile(*totalCount, allLines);
		} else if (strcmp(argv[1], "-d") == 0) {
			deleteTask(totalCount, argv[2], allLines);
			writeFile(*totalCount, allLines);
		} else if (argv[1][0] == '+' || argv[1][0] == '-') {
			incrementTime(*totalCount, allLines, argv[1], argv[2]);
			writeFile(*totalCount, allLines);
		} else {
			printHelp(argv[0], totalCount, allLines);
		}
	} else if (argc > 3) {
		printHelp(argv[0], totalCount, allLines);
	}
	printTimeSheet(*totalCount, allLines);
}

void setFileName() {
	const char* file_name = ".timecop.txt";
	const char* homedir;
	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	file = (char *)calloc(1,
			(strlen(homedir) + strlen(file_name) + 2) * sizeof(char));
	strcat(file, homedir);
	strcat(file, "/");
	strcat(file, file_name);
}


int main(int argc, char **argv) {
	if (argc >= 2 && strcmp(argv[1], "-t") == 0) {
		int argc2 = argc-1;
		testing::InitGoogleTest(&argc2, argv+1);
		return RUN_ALL_TESTS();
	}

	int totalCount = 0;
	line** allLines = (line **)calloc(100, sizeof(line*));
	setFileName();
	readFile(&totalCount, allLines);
	parseCommand(argc, argv, &totalCount, allLines);
	cleanup(&totalCount, allLines);
}

