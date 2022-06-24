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
#include <chrono>

#include "EDAoogleHttpRequestHandler.h"

using namespace std;
using namespace chrono;
using namespace filesystem;

/**
 * @brief Class constructor
 *
 * @param homePath
 */

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    // Search the correct path using the name of the folder.
    searchEngine.index(homePath);
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
    
        unordered_set<string> results;

        time_point<steady_clock> timestamp = steady_clock::now();

        searchEngine.search(searchString, results);

        unsigned long durationUs = duration_cast<microseconds>(steady_clock::now() - timestamp).count();
        float searchTime = (float)(durationUs / 1000000.0f);

        // Print search results
        responseString += "<div class=\"results\">" + to_string(results.size()) +
                          " results (" + to_string(searchTime) + " seconds):</div>";
        for (auto &result : results)
        {
            responseString += "<div class=\"result\"><a href=\"" 
                           + result + "\">" + result + "</a></div>";

        }

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
