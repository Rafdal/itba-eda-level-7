/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl. Editado por Fernanda Cattaneo y Rafael Dalzotto
 * @brief EDAoggle search engine
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <regex>
#include <filesystem>
#include <fstream>
#include <list>
#include <string>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <array>

#include "EDAoogleHttpRequestHandler.h"

using namespace std;
using namespace filesystem;
namespace fs = filesystem;

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
    void print(string tab = "", char index = '@', string word = "") // JUST FOR TESTING
    {
        cout << tab << '[' << index << ']' << endl;
        word += index;
        for(auto& p : pages)
            cout << tab << " " << index << "->page = " << p << " (\"" << word << "\")" << endl;

        tab += ' ';
        int i=0;
        for(auto& n : childs)
        {
            if(n != NULL)
            {
                n->print(tab, indexToChar(i), word);
            }
            i++;
        }
    }
    array<TrieNode*, TRIE_INDEX_SIZE> childs;
    unordered_set<string> pages;
};

struct TrieRoot{
    TrieRoot()
    {
        for(auto& n : childs)
            n = NULL;
    }
    void print() // JUST FOR TESTING
    {
        int i=0;
        for(auto& n : childs)
        {
            if(n != NULL)
            {
                n->print("", indexToChar(i));
            }
            i++;
        }
    }
    array<TrieNode*, TRIE_INDEX_SIZE> childs;
};

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

/**
 * @brief Converts file into a text data
 *
 * @param filePath - file to convert
 * @param text - Destination text
 */

bool getTextFromFile(const char *filePath, string &text)
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

static TrieRoot trieRoot;

void insertPageInTrie(string& word, string& page)
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

void getPagesFromTrie(string& word, vector<string>& pages)
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

void processFile(path filepath, list<string>& words)
{
    string text, word;
    const regex pattern("\\<.*?>"); // remove generic html tags

    words.clear();

    if (getTextFromFile(filepath.c_str(), text))
    {
        text = regex_replace(text, pattern, "");

        auto it = string::iterator();
        auto itAux = string::iterator();
        bool wordEnding = false;

        // Search html characters
        for (it = text.begin(); it != text.end(); it++)
        {
            if (*it == '&')
            {
                itAux = it;
                it++;
                if (*it == '#')
                {
                    string code = "";
                    it++;
                    for (; it != text.end() && (it - itAux) < 8; it++)
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

/**
 * @brief Class constructor
 *
 * @param homePath
 */

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    // Search the correct path using the name of the folder.
    path local = current_path().parent_path();
    local /= homePath;
    local /= "wiki"; // subdirectory

    for(const auto& dirEntry : directory_iterator(local))
    {
        if(dirEntry.path().extension().compare(".html") == 0) // extension match
        {
            cout << "Processing " << dirEntry.path().filename() << endl;

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

    // trieRoot.print();

    string query = "muy";
    vector<string> pages;
    getPagesFromTrie(query, pages);

    cout << pages.size() << " ";
    cout << "search results for \"" << query << "\"" << endl;
    for(auto& p : pages)
    {
        cout << "page: " << p << endl;
    }

}

EDAoogleHttpRequestHandler::~EDAoogleHttpRequestHandler()
{
    for(auto& n : trieRoot.childs)
        delete n;
}

/**
 * @brief When this method is called allows to obtain a list of pages according to the searched word 
 *
 * @param url
 * @param arguments
 * @param response
 */

bool EDAoogleHttpRequestHandler::handleRequest(string url,
                                               HttpArguments arguments,
                                               vector<char> &response)
{
    auto it = string::iterator();
    string searchPage = "/search";
    if (url.substr(0, searchPage.size()) == searchPage)
    {
        string searchString;
        if (arguments.find("q") != arguments.end())
            searchString = arguments["q"];
            
        for_each(searchString.begin(), searchString.end(), [](char & c){
            c = ::tolower(c);
        });
        for (it = searchString.begin(); it != searchString.end(); it++)
        {
            if ( *it < '0' || *it > 'z')
            {
                 searchString.erase(remove(searchString.begin(), searchString.end(), *it), searchString.end());        
            }
            else if (*it > '9' && *it < 'A')
            {
                searchString.erase(remove(searchString.begin(), searchString.end(), *it), searchString.end());
            }
            else if (*it > 'Z' && *it < 'a')
            {
                searchString.erase(remove(searchString.begin(), searchString.end(), *it), searchString.end());
            }
        }

        cout << arguments["q"] << endl;
        // Header
        string responseString = string("<!DOCTYPE html>\
<html>\
\
<head>\
    <meta charset=\"utf-8\" />\
    <title>EDAoogle</title>\
    <link rel=\"preload\" href=\"https://fonts.googleapis.com\" />\
    <link rel=\"preload\" href=\"https://fonts.gstatic.com\" crossorigin />\
    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@400;800&display=swap\" rel=\"stylesheet\" />\
    <link rel=\"preload\" href=\"../css/style.css\" />\
    <link rel=\"stylesheet\" href=\"../css/style.css\" />\
</head>\
\
<body>\
    <article class=\"edaoogle\">\
        <div class=\"title\"><a href=\"/\">EDAoogle</a></div>\
        <div class=\"search\">\
            <form action=\"/search\" method=\"get\">\
                <input type=\"text\" name=\"q\" value=\"" +
                        searchString + "\" autofocus>\
            </form>\
        </div>\
        ");
    
        float searchTime = 0.1F;
        vector<string> results;
        getPagesFromTrie(searchString, results); //Funcion de busqueda.
        // Print search results
        responseString += "<div class=\"results\">" + to_string(results.size()) +
                          " results (" + to_string(searchTime) + " seconds):</div>";
        for (auto &result : results)
            responseString += "<div class=\"result\"><a href=\"" +
                              result + "\">" + result + "</a></div>";

        // Trailer
        responseString += "    </article>\
</body>\
</html>";

        response.assign(responseString.begin(), responseString.end());

        return true;
    }
    else
        return serve(url, response);

    return false;
}
