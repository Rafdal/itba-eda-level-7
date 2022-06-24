/**
 * @file SearchEngine.h
 * @author Fernanda Cattaneo y Rafael Dalzotto
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#define TRIE_INDEX_SIZE 36 

#include <iostream>
#include <filesystem>
#include <fstream>
#include <list>
#include <string>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <array>
#include <chrono>
#include <sstream>

struct TrieNode{
    TrieNode()
    {
        for(auto& n : childs)
            n = NULL;
    }
    ~TrieNode()
    {
        for(auto& n : childs)
            delete n;
    }
    std::array<TrieNode*, TRIE_INDEX_SIZE> childs;
    std::unordered_set<std::string> pages;
};

struct TrieRoot{
    TrieRoot()
    {
        for(auto& n : childs)
            n = NULL;
    }
    ~TrieRoot()
    {
        for(auto& n : childs)
            delete n;
    }
    std::array<TrieNode*, TRIE_INDEX_SIZE> childs;
};

class SearchEngine
{
private:
    TrieRoot trieRoot;
    bool getTextFromFile(const char *filePath, std::string &text);
    void insertPageInTrie(const std::string &word, std::string &page);
    void getPagesFromTrie(std::string& word, std::unordered_set<std::string>& pages);
    void processFile(std::filesystem::path filepath, std::unordered_set<std::string>& words);

public:
    SearchEngine(){}

    void index(std::string& homePath);
    void search(std::string& searchQuery, std::unordered_set<std::string>& pages);
};




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
        return ('a' + idx - 10);
    }
    else if(idx >= 0 && idx < 10)
    {
        return '0' + idx;
    }
    return '\0';
}

#endif