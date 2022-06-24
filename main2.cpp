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

namespace fs = filesystem;
using namespace filesystem;
using namespace std;

//Map that conteins the html entities and the character in ascii.
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


void buildIndex(string homePath)
{
    // build index
    // Search the correct path using the name of the folder.
    path local = current_path().parent_path();
    local /= homePath;
    local /= "wiki";
    local /= "Actor.html";

    string text;
    const regex pattern("\\<.*?>");

    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

    if (getTextFromFile(local.c_str(), text))
    {
        text = regex_replace(text, pattern, "");

        // convert all string to lower characters.
        // boost::algorithm::to_lower(line);

        auto it = string::iterator();
        auto itAux = string::iterator();
        bool wordEnding = false;

        // Search useless characters
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

            if (isalpha(*it) || isdigit(*it) || *it == '&' || *it == '#' || *it == ';')
            {
                if(isalpha(*it) || isdigit(*it))
                {
                    char c = tolower(*it);
                }
                wordEnding = true;
            }
            else if (wordEnding)
            {
                cout << ' '; // fin / inicio de palabra
                wordEnding = false;
            }
        }

        // ASCO DESPUES CAMBIAR ESTOY MUY HARDCODEADO
/*         int lineCount = 0;
        for (auto &c : text)
        {

        } */
    }
    else
    {
        cout << "asd\n";
    }
}

int main()
{
    buildIndex("www");

    return 0;
}