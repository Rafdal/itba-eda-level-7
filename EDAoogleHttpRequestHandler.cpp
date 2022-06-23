/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl
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

#include "EDAoogleHttpRequestHandler.h"

using namespace std;
using namespace filesystem;

namespace fs = std::filesystem;

// Data type: list of text lines
typedef list<string> Text;

static bool getTextFromFile(const char* filePath, Text& text);
static bool getText(const string &s, Text &text);

/**
 * @brief Converts a string into a Text (list of lines)
 *
 * @param s - string to convert
 * @param text - Destination text
 */

bool getText(const string &s, Text &text)
{
    text.clear();

    string::size_type pos = 0;
    string::size_type prev = 0;
    while ((pos = s.find('\n', prev)) != string::npos)
    {
        text.push_back(s.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    text.push_back(s.substr(prev));

    return true;
}

/**
 * @brief Converts file into a text data
 *
 * @param filePath - file to convert
 * @param text - Destination text
 */

bool getTextFromFile(const char* filePath, Text& text)
{
    ifstream file(filePath);

    if (file.is_open())
    {
        cout << "open\n";
        file.seekg(0, ios::end);
        int fileSize = file.tellg() > 1000000 ? 1000000 : (int)file.tellg();
        string fileData(fileSize, ' ');
        file.seekg(0);
        file.read(&fileData[0], fileSize);

        return !file.fail() && getText(fileData.c_str(), text);
    }

    return false;
}

/**
 * @brief Contructor
 *
 * @param s - string
 */

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    //Search the correct path using the name of the folder.
    path local = fs::current_path().parent_path();
    local /= homePath;
    local /= "wiki";
    local /= "Actor.html";

    Text text;
    const regex pattern("\\<.*?\\>");

    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    
    if(getTextFromFile(local.c_str(), text))
    {
        for(auto& line : text)
        {
            line = regex_replace(line, pattern, "");

            //convert all string to lower characters.
            //boost::algorithm::to_lower(line);

            //Search useless characters
            for(auto& c : line)
            {
                if(isalpha(c) || isdigit(c) || c == '&' || c == '#' || c == ';')
                {
                    cout << c;
                }
                else
                    cout << '\n'; // fin / inicio de palabra
            }
            cout << '\n'; // fin / inicio de palabra
        }
    }
    else
    {
        cout << "asd\n";
    }

    return ;
}

bool EDAoogleHttpRequestHandler::handleRequest(string url,
                                               HttpArguments arguments,
                                               vector<char> &response)
{

    string searchPage = "/search";
    if (url.substr(0, searchPage.size()) == searchPage)
    {
        string searchString;
        if (arguments.find("q") != arguments.end())
            searchString = arguments["q"];

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

        // YOUR JOB: fill in results
        float searchTime = 0.1F;
        vector<string> results;

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
