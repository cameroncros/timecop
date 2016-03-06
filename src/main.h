/*
 * main.h
 *
 *  Created on: 16 Feb 2016
 *      Author: cameron
 */

#ifndef MAIN_H_
#define MAIN_H_

typedef struct {
	char *name;
	long timeStarted;
	long timeSpent;
} line;

void parseCommand(int argc, char** argv, int *totalCount, line** allLines);
void printHelp(char *str, int *totalCount, line** allLines);
line* processLine(char *str);
void readFile(int *totalCount, line** allLines);
void cleanup(int *totalCount, line** allLines);
void writeFile(int totalCount, line** allLines);
void printTimeSheet(int totalCount, line** allLines);
void deleteTimers(int *totalCount);
void resetTimers(int totalCount, line** allLines);
void addTask(int *totalCount, char* name, line** allLines);
void deleteTaskIndex(int *totalCount, int index, line** allLines);
int getValidIndex(const char *id, int totalCount);
void deleteTask(int *totalCount, char* id, line** allLines);
void startTimer(int totalCount, char** argv, line** allLines);
void pauseTimers(int totalCount, line** allLines);
void incrementTime(int totalCount, line** allLines, char*incAmount, char *id);
int readStandardInput(char** argv);
void interactiveConsole(int *totalCount, line** allLines);
void parseCommand(int argc, char** argv, int *totalCount, line** allLines);

#endif /* MAIN_H_ */
