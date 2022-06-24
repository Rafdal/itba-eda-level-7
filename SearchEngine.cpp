
/**
 * 
 * @file SearchEngine.cpp
 * @author Fernanda Cattaneo y Rafael Dalzotto
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

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

#include "SearchEngine.h"

using namespace std;
using namespace filesystem;
using namespace chrono;
namespace fs = filesystem;

// Map that contains the html entities and its normalized character in ascii.
unordered_map<string, string> htmlCode({
    {"192", "a"},
    {"193", "a"},
    {"225", "a"},
    {"226", "a"},
    {"201", "e"},
    {"200", "e"},
    {"233", "e"},
    {"205", "i"},
    {"237", "i"},
    {"243", "o"},
    {"211", "o"},
    {"218", "u"},
    {"252", "u"},
    {"250", "u"},
    {"209", "n"},
    {"241", "n"},
});

SearchEngine::~SearchEngine()
{
    for(auto& n : trieRoot.childs)
        delete n;
}


void SearchEngine::index(string homePath)
{
    path local = current_path().parent_path();
    local /= homePath;
    local /= "wiki"; // subdirectory
    
    long pageCount = 0;
    for(const auto& dirEntry : directory_iterator(local))
    {
        if(dirEntry.path().extension().compare(".html") == 0) // extension match
        {
            cout << "Indexing page " << pageCount++ << " of 1284 " << dirEntry.path().filename() << endl;

            list<string> words;
            processFile(dirEntry.path(), words);

            string pageName = dirEntry.path().filename().replace_extension("").string();
            for(auto& w : words)
            {
                // cout << "word: \'" << w << "\' page:" << pageName << endl;
                insertPageInTrie(w, pageName);
            }

            // cout << endl;
        }
    }
}

/**
 * @brief Converts file into a text data
 *
 * @param filePath - file to convert
 * @param text - Destination text
 */

bool SearchEngine::getTextFromFile(const char *filePath, string &text)
{
    ifstream file(filePath);
    string line;
    
    if(!file.is_open())
        return false;

    text.clear();
    while (getline(file, line))
    {
        text += line;
    }
    return true;
}

/**
 * @brief Inserts a page into the trie after the last character of the loaded word.
 *
 * @param word - loaded word
 * @param page - website address.
 */

void SearchEngine::insertPageInTrie(string& word, string& page)
{
    if(word.size() == 0)
        return; // null query
        
    queue<int> wordIndex; // build insert index
    for(auto& c : word)
    {
        int idx = charToIndex(c);
        if(idx >= 0 && idx < TRIE_INDEX_SIZE)
            wordIndex.push(idx);
        else 
        {
            throw logic_error("ERROR en TRIE: caracter fuera de rango\n");
            return;
        }
    }

    if(trieRoot.childs[wordIndex.front()] == NULL)
        trieRoot.childs[wordIndex.front()] = new TrieNode;

    TrieNode* node = trieRoot.childs[wordIndex.front()];
    wordIndex.pop();

    bool finished = false;
    while (!finished)
    {
        if(wordIndex.empty()) // iteration completed
        {
            node->pages.insert(page);
            finished = true;
        }
        else // to to the next node
        {
            if(node->childs[wordIndex.front()] == NULL)
                node->childs[wordIndex.front()] = new TrieNode;

            node = node->childs[wordIndex.front()];
            wordIndex.pop();
        }
    }
}

/**
 * @brief function that allows to obtain a web page according to the searched word 
 *
 * @param word - searched word
 * @param pages - Vector that contains all the pages found
 */

void SearchEngine::getPagesFromTrie(string& word, vector<string>& pages)
{
    if(word.size() == 0)
        return; // null search query
        
    queue<char> query;
    for(auto&c : word)
        query.push(c);

    TrieNode* node = trieRoot.childs[charToIndex(query.front())];
    query.pop();

    if(node == NULL)
        return; // key doesnt exist in root (very rare)

    bool finished = false;
    while (!finished)
    {
        if(query.empty()) // se alcanzo la palalbra completa
        {
            // TODO: Agregar aca interseccion de conjuntos de paginas
 
            pages.clear();
            pages.assign(node->pages.begin(), node->pages.end());
            finished = true;
        }
        else // get the next node
        {
            node = node->childs[charToIndex(query.front())];
            query.pop();

            if(node == NULL) // child node does not exist
                finished = true;
        }
    }
}

/**
 * @brief function that allows to obtain a text from a file. 
 *
 * @param filepath - file direction
 * @param word 
 */

void SearchEngine::processFile(path filepath, list<string>& words)
{
    string textAux, word;
    string text = "";

    if (!getTextFromFile(filepath.c_str(), text))
    {
        throw logic_error("No se puede abrir el archivo\n");
    }

    auto it = string::iterator();
    auto itAux = string::iterator();
    bool wordEnding = false;

    // Search useless characters, words between <> and words that describes the style.
    bool tagFlag = false;
    bool styleOff = true;
    string buffer = "";
    for (it = text.begin(); it != text.end(); it++)
    {
        switch (*it)
        {
        case '<':
            tagFlag = true;
            continue;
            break;
        case '>':
            tagFlag = false;
            buffer.clear();
            continue;
            break;
        }

        if(tagFlag)
        {
            buffer += *it;
            if (buffer == "/style")
            {
                styleOff = true;
                buffer.clear();
            }
            if (buffer == "style")
            {
                styleOff = false;
                buffer.clear();
            }
        }

        if (!styleOff)
        {
            continue;
        }

        if (*it == '&')
        {
            itAux = it;
            it++;
            if (*it == '#')
            {
                string code = "";
                it++;
                for (; it != text.end() && (it - itAux) < 9; it++)
                {
                    if (*it == ';')
                    {
                        text.replace(itAux, it + 1, htmlCode[code]);
                        it = itAux;
                        break;
                    }
                    code += *it;
                }
            }
        }

        if(!tagFlag && styleOff)
        {
            if (isalpha(*it) || isdigit(*it) || *it == '&' || *it == '#' || *it == ';')
            {
                if(isalpha(*it) || isdigit(*it))
                {
                    word += tolower(*it);
                }
                wordEnding = true;
            }
            else if (wordEnding)
            {
                if(word.size() > 0)
                {
                    words.push_back(word);
                    word.clear();
                }
                wordEnding = false;
            }
        }
    }
}

void SearchEngine::search(std::string searchQuery, std::vector<std::string>& pages)
{
    processFile(path filepath, list<string>& words)
}