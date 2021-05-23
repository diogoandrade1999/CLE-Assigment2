/**
 * \file  convertchar.h
 * \brief File with shared functions of convert char
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#ifndef CONVERT_CHAR_H
#define CONVERT_CHAR_H

/**
 * \brief Converts Special char in Normal char
 * \param buf Array with chars
 * \param i Index of readed char of array
 * \return Normal char
 */
unsigned char convertChar(unsigned char *buf, int *i);

/**
 * \brief Converts Special vowel char in Normal vowel char
 * \param c Special vowel char
 * \return Normal vowel char
 */
unsigned char getVowel(int c);

/**
 * \brief Check if the char is a vowel
 * \param c Char to be checked
 * \return 0 if is not a vowel and 1 if is a vowel
 */
int isVowel(char c);

/**
 * \brief Check if the char is a space
 * \param c Char to be checked
 * \return 0 if is not a space and 1 if is a space
 */
int isSpace(char c);

/**
 * \brief Check if the char is a separation
 * \param c Char to be checked
 * \return 0 if is not a separation and 1 if is a separation
 */
int isSeparation(char c);

/**
 * \brief Check if the char is a punctuation
 * \param c Char to be checked
 * \return 0 if is not a punctuation and 1 if is a punctuation
 */
int isPunct(char c);

/**
 * \brief Check if the char is a merge char
 * \param c Char to be checked
 * \return 0 if is not a merge char and 1 if is a merge char
 */
int isMerge(char c);

/**
 * \brief Check if the char is a inside of the word
 * \param c Char to be checked
 * \return 0 if is not a inside of the word and 1 if is a inside of the word
 */
int inWord(char c);

/**
 * \brief Check if the char is a consonant
 * \param c Char to be checked
 * \return 0 if is not a consonant and 1 if is a consonant
 */
int isConsonant(char c);

#endif