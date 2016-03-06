/*
 * test.c
 *
 *  Created on: 17 Feb 2016
 *      Author: cameron
 */

#include "../src/main.h"
#include <gtest/gtest.h>

TEST (processLine, validValues) {
	EXPECT_FALSE(processLine((char *)"asdf") == NULL);
	EXPECT_FALSE(processLine((char *)"asdfasdf 10") == NULL);
	EXPECT_FALSE(processLine((char *)"asdfasdf 10 10") == NULL);
	EXPECT_FALSE(processLine((char *)"asdf\n") == NULL);
	EXPECT_FALSE(processLine((char *)"asdfasdf 10\n") == NULL);
	EXPECT_FALSE(processLine((char *)"asdfasdf 10 10\n") == NULL);

}

TEST (processLine, invalidValues) {
	EXPECT_EQ(NULL, processLine((char *)"asdf asdf"));
	EXPECT_EQ(NULL, processLine((char *)"asdf aasdf asdfs"));
	EXPECT_EQ(NULL, processLine(NULL));
}

TEST (getValidIndex, validIndexes) {
	EXPECT_EQ(0, getValidIndex("1", 1));
	EXPECT_EQ(1, getValidIndex("2", 3));
}

TEST (getValidIndex, invalidIndexes) {
	EXPECT_EQ(-1, getValidIndex("1", 0));
	EXPECT_EQ(-1, getValidIndex("0", 3));
	EXPECT_EQ(-1, getValidIndex("4", 3));
	EXPECT_EQ(-1, getValidIndex("-2", 3));
}

TEST (readStandardInput, validInput) {
	char** argv = (char **)calloc(3, sizeof(char*));
	char msg[] = "asdf";
	argv[1] = msg;
	EXPECT_EQ(2, readStandardInput(argv));
	char msg2[] = "asdfasdf 10";
	argv[1] = msg2;
	EXPECT_EQ(3, readStandardInput(argv));
}

TEST (readStandardInput, invalidInput) {
	char** argv = (char **)calloc(3, sizeof(char*));
	char msg[] = "asdfasdf 10 10";
	argv[1] = msg;
	EXPECT_EQ(4, readStandardInput(argv));

	char msg2[] = "";
	argv[1] = msg2;
	EXPECT_EQ(1, readStandardInput(argv));
}
