#pragma once
#include "parser.h"

TCHAR** stringParser(const TCHAR* string, TCHAR delimiter)
{
    if (string == NULL)
        return NULL;

    int spaces = 0;
    int maxWordSize = 0;
    int currentWordSize = 0;

    for (int i = 0; TRUE; i++)
    {
        if (string[i] == delimiter)
        {
            spaces++;

            if (currentWordSize >= maxWordSize)
            {
                maxWordSize = currentWordSize;
                currentWordSize = 0;
            }
            continue;
        }
        else if (string[i] == _T('\0'))
        {
            if (currentWordSize >= maxWordSize)
            {
                maxWordSize = currentWordSize;
                currentWordSize = 0;
            }

            break;
        }
        currentWordSize++;
    }

    int arraySize = spaces + 2; //words = spaces + 1 (+ 1 for NULL)

    TCHAR** parsedStrings = calloc(arraySize, sizeof(TCHAR*));

    if (parsedStrings == NULL)
        return NULL;

    for (int i = 0; i < arraySize; i++)
    {
        parsedStrings[i] = calloc(maxWordSize, sizeof(TCHAR));

        if (parsedStrings[i] == NULL)
        {
            _tprintf(L"Allocation Error!\n");

            for (int j = 0; j < i; j++)
                free(parsedStrings[j]);

            return NULL;
        }
    }

    if (arraySize == 2) //Only 1 word + NULL
    {
        for (int i = 0; string[i] != _T('\0'); i++)
        {
            parsedStrings[0][i] = string[i];
        }

        parsedStrings[0][_tcslen(string)] = _T('\0');
        parsedStrings[1] = NULL;
    }
    else if (arraySize < 2) //something went HORRIBLY WRONG!
        return NULL;
    else
    {
        int i = 0;

        for (int y = 0; TRUE; y++)
        {
            for (int x = 0; TRUE; x++)
            {
                if (string[i] == delimiter || string[i] == L'\0')
                {
                    parsedStrings[y][x] = _T('\0');
                    break;
                }
                else
                    parsedStrings[y][x] = string[i];

                i++;
            }

            if (string[i] == _T('\0'))
                break;

            i++;
        }
    }

    parsedStrings[arraySize - 1] = NULL;

    return parsedStrings;
}