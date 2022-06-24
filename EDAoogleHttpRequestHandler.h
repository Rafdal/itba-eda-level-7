/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl. Editado por Fernanda Cattaneo y Rafael Dalzotto
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EDAOOGLEHTTPREQUESTHANDLER_H
#define EDAOOGLEHTTPREQUESTHANDLER_H

#include "ServeHttpRequestHandler.h"

#define TRIE_INDEX_SIZE 36 

class EDAoogleHttpRequestHandler : public ServeHttpRequestHandler
{
public:
    EDAoogleHttpRequestHandler(std::string homePath);

    bool handleRequest(std::string url, HttpArguments arguments, std::vector<char> &response);
};

bool getTextFromFile(const char *filePath, std::string &text);

void getPagesFromTrie(std::string& word, std::vector<std::string>& pages);
void insertPageInTrie(std::string& word, std::vector<std::string>& pages);


inline int charToIndex(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - 'a' + 10;
    }
    else if(c >= '0' && c <= '9')
    {
        return c - '0';
    }
    return -1;
}

inline char indexToChar(int idx)
{
    if(idx >= 10 && idx < TRIE_INDEX_SIZE)
    {
        return 'a' + idx;
    }
    else if(idx >= 0 && idx < 10)
    {
        return '0' + idx;
    }
    return '\0';
}

#endif
