#ifndef CONVERT_CHAR_H
#define CONVERT_CHAR_H

unsigned char convertChar(unsigned char *buf, int *i);
unsigned char getVowel(int c);
int isVowel(char c);
int isSpace(char c);
int isSeparation(char c);
int isPunct(char c);
int isMerge(char c);
int inWord(char c);
int isConsonant(char c);

#endif