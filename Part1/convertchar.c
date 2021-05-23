/**
 * \file  convertchar.c
 * \brief File with implementation of functions of convert char
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#include "convertchar.h"
#include <ctype.h>

// Char of Specials Vowels
unsigned char aVowel[8] = {0xa0, 0xa1, 0xa2, 0xa3, 0x80, 0x81, 0x82, 0x83};
unsigned char eVowel[6] = {0xa8, 0xa9, 0xaa, 0x88, 0x89, 0x8a};
unsigned char iVowel[4] = {0xac, 0xad, 0x8c, 0x8d};
unsigned char oVowel[8] = {0xb2, 0xb3, 0xb4, 0xb5, 0x92, 0x93, 0x94, 0x95};
unsigned char uVowel[4] = {0xb9, 0xba, 0x99, 0x9a};

/**
 * \brief Converts Special char in Normal char
 * \param buf Array with chars
 * \param i Index of readed char of array
 * \return Normal char
 */
unsigned char convertChar(unsigned char *buf, int *i)
{
    unsigned char c;
    c = buf[*i];
    if (c == 0xc3)
    {
        (*i)++;
        c = buf[*i];
        if (c == 0x87 || c == 0xa7)
            c = 'c';
        else
            c = getVowel(c);
    }
    else if (c == 0xe2)
    {
        (*i)++;
        c = buf[*i];
        if (c == 0x80)
        {
            (*i)++;
            c = buf[*i];
            if (c == 0x9c || c == 0x9d)
                c = '-';
            else if (c == 0x93 || c == 0xa6)
                c = '.';
            else if (c == 0x98 || c == 0x99)
                c = 0x27;
        }
    }
    else if (c == 0xc2)
    {
        (*i)++;
        c = buf[*i];
        if (c == 0xab || c == 0xbb)
            c = ' ';
    }
    return c;
}

/**
 * \brief Converts Special vowel char in Normal vowel char
 * \param c Special vowel char
 * \return Normal vowel char
 */
unsigned char getVowel(int c)
{
    for (int i = 0; i < 8; i++)
        if (c == aVowel[i])
            return 'a';
    for (int i = 0; i < 6; i++)
        if (c == eVowel[i])
            return 'e';
    for (int i = 0; i < 4; i++)
        if (c == iVowel[i])
            return 'i';
    for (int i = 0; i < 8; i++)
        if (c == oVowel[i])
            return 'o';
    for (int i = 0; i < 4; i++)
        if (c == uVowel[i])
            return 'u';
    return 0;
}

/**
 * \brief Check if the char is a vowel
 * \param c Char to be checked
 * \return 0 if is not a vowel and 1 if is a vowel
 */
int isVowel(char c)
{
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
        return 1;
    if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U')
        return 1;
    return 0;
}

/**
 * \brief Check if the char is a space
 * \param c Char to be checked
 * \return 0 if is not a space and 1 if is a space
 */
int isSpace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == 0xa)
        return 1;
    return 0;
}

/**
 * \brief Check if the char is a separation
 * \param c Char to be checked
 * \return 0 if is not a separation and 1 if is a separation
 */
int isSeparation(char c)
{
    if (c == '-' || c == '"' || c == '[' || c == ']' || c == '(' || c == ')')
        return 1;
    return 0;
}

/**
 * \brief Check if the char is a punctuation
 * \param c Char to be checked
 * \return 0 if is not a punctuation and 1 if is a punctuation
 */
int isPunct(char c)
{
    if (c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c == '!')
        return 1;
    return 0;
}

/**
 * \brief Check if the char is a merge char
 * \param c Char to be checked
 * \return 0 if is not a merge char and 1 if is a merge char
 */
int isMerge(char c)
{
    if (c == 0x27)
        return 1;
    return 0;
}

/**
 * \brief Check if the char is inside of the word
 * \param c Char to be checked
 * \return 0 if is not a inside of the word and 1 if is a inside of the word
 */
int inWord(char c)
{
    if (isalnum(c) || isMerge(c) || c == '_')
        return 1;
    return 0;
}

/**
 * \brief Check if the char is a consonant
 * \param c Char to be checked
 * \return 0 if is not a consonant and 1 if is a consonant
 */
int isConsonant(char c)
{
    if (inWord(c))
        if (!isVowel(c) && !isdigit(c) && !isMerge(c) && c != '_')
            return 1;
    return 0;
}