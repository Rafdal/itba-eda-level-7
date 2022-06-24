
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

#include "SearchEngine.h"

using namespace std;
using namespace filesystem;
using namespace chrono;

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
 * @brief function to separate the searched words between spaces 
 *
 * @param str - string to separate
 * @param output - separate words in a queue
 */
static void splitString(string &str, queue<string> &output)
{
    stringstream ss(str);
    string item;
    unordered_set<string> searchKeywordsSet; // usamos un set para evitar palabras repetidas

    while (getline(ss, item, ' '))
        searchKeywordsSet.insert(item);

    for (auto &w : searchKeywordsSet)
        output.push(w);
}

/**
 * @brief intersection between two sets of strings
 *
 * @param a - set of strings
 * @param b - set of strings 
 * @param o - output intersection
 */
static void unordered_set_intersection(unordered_set<string> &a, unordered_set<string> &b, unordered_set<string> &o)
{
    for (auto i = a.begin(); i != a.end(); i++)
    {
        if (b.find(*i) != b.end())
            o.insert(*i);
    }
}

/**
 * @brief index all the pages
 *
 * @param homePath
 * 
 */
void SearchEngine::index(string& homePath)
{
    path local = current_path();
    local /= homePath;
    local /= "wiki"; // subdirectory
    
    long pageCount = 0;
    for(const auto& dirEntry : directory_iterator(local))
    {
        if(dirEntry.path().extension().compare(".html") == 0) // extension match
        {
            cout << "Indexing page " << pageCount++ << " of 1284 " << dirEntry.path().filename() << endl;

            unordered_set<string> words;
            processFile(dirEntry.path(), words);

            string pageName = dirEntry.path().filename().string();
            for(auto& w : words)
            {
                insertPageInTrie(w, pageName);
            }
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

void SearchEngine::insertPageInTrie(const string &word, string &page)
{
    if (word.size() == 0)
        return; // null query

    queue<int> wordIndex; // build insert index
    for (auto &c : word)
    {
        int idx = charToIndex(c);
        if (idx >= 0 && idx < TRIE_INDEX_SIZE)
            wordIndex.push(idx);
        else
        {
            throw logic_error("ERROR en TRIE: caracter fuera de rango\n");
            return;
        }
    }

    if (trieRoot.childs[wordIndex.front()] == NULL)
        trieRoot.childs[wordIndex.front()] = new TrieNode;

    TrieNode *node = trieRoot.childs[wordIndex.front()];
    wordIndex.pop();

    bool finished = false;
    while (!finished)
    {
        if (wordIndex.empty()) // iteration completed
        {
            node->pages.insert(page);
            finished = true;
        }
        else // to to the next node
        {
            if (node->childs[wordIndex.front()] == NULL)
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

void SearchEngine::getPagesFromTrie(string &word, unordered_set<string> &pages)
{
    if (word.size() == 0)
        return; // null search query

    queue<char> query;
    for (auto &c : word)
        query.push(c);

    TrieNode *node = trieRoot.childs[charToIndex(query.front())];
    query.pop();

    if (node == NULL)
        return; // key doesnt exist in root (very rare)

    bool finished = false;
    while (!finished)
    {
        if (query.empty()) // se alcanzo la palalbra completa
        {
            pages.clear();
            pages.insert(node->pages.begin(), node->pages.end());
            finished = true;
        }
        else // get the next node
        {
            node = node->childs[charToIndex(query.front())];
            query.pop();

            if (node == NULL) // child node does not exist
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

void SearchEngine::processFile(path filepath, unordered_set<string> &words)
{
    string textAux, word;
    string text = "";

    if (!getTextFromFile(filepath.c_str(), text))
    {
        throw logic_error("No se puede abrir el archivo\n");
    }

    // convert all string to lower characters.
    // boost::algorithm::to_lower(line);

    auto it = string::iterator();
    auto itAux = string::iterator();
    bool wordEnding = false;

    // Search useless characters
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

        if (tagFlag)
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

        if (!tagFlag && styleOff)
        {
            if (isalpha(*it) || isdigit(*it) || *it == '&' || *it == '#' || *it == ';')
            {
                if (isalpha(*it) || isdigit(*it))
                {
                    word += tolower(*it);
                }
                wordEnding = true;
            }
            else if (wordEnding)
            {
                if (word.size() > 0)
                {
                    words.insert(word);
                    word.clear();
                }
                wordEnding = false;
            }
        }
    }
}

/**
 * @brief seacrh the pages that contains the searched word
 * 
 * @param searchQuery - searched word
 * @param results - output
 */
void SearchEngine::search(std::string& searchQuery, std::unordered_set<std::string>& results)
{
    queue<string> searchKeywords;
    splitString(searchQuery, searchKeywords); // split words

    cout << "search:\n";
    if (searchKeywords.size() > 0)
    {
        getPagesFromTrie(searchKeywords.front(), results); // gather first results
        cout << '\"' << searchKeywords.front() << '\"' << endl;
        searchKeywords.pop();

        while (searchKeywords.size() > 0)
        {
            unordered_set<string> auxResults, intersection;
            cout << '\"' << searchKeywords.front() << '\"' << endl;
            getPagesFromTrie(searchKeywords.front(), auxResults); // gather first results
            searchKeywords.pop();

            unordered_set_intersection(results, auxResults, intersection);

            if (intersection.empty())
            {
                results.clear();
                break; // No se pudo encontrar nada que coincida con los criterios de busqueda
            }

            results = intersection;
        }
    }
}