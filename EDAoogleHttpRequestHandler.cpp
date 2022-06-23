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

#include "EDAoogleHttpRequestHandler.h"

using namespace std;
using namespace filesystem;

namespace fs = std::filesystem;

EDAoogleHttpRequestHandler::EDAoogleHttpRequestHandler(string homePath) : ServeHttpRequestHandler(homePath)
{
    const regex pattern("\\<.*?\\>");

    path local = fs::current_path().parent_path();
    local /= homePath;
    local /= "index.html";
    error_code err;

    cout << local.c_str() << endl;
    uintmax_t size = fs::file_size(local, err);
    if(err.value())
        cout << "err\n";
    else
        cout << "NoErr\n";

    cout << "size: " << size << endl;

    // Use regex_replace function in regex
    // to erase every tags enclosed in <>
    // s = regex_replace(s, pattern, "");
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
