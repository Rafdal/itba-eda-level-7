/**
 * @file EDAoogleHttpRequestHandler.h
 * @author Marc S. Ressl. Editado por Fernanda Cattaneo y Rafael Dalzotto
 * @brief EDAoggle Request Handler
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EDAOOGLEHTTPREQUESTHANDLER_H
#define EDAOOGLEHTTPREQUESTHANDLER_H

#include "ServeHttpRequestHandler.h"
#include "SearchEngine.h"

class EDAoogleHttpRequestHandler : public ServeHttpRequestHandler
{
private:
    SearchEngine searchEngine;

public:
    EDAoogleHttpRequestHandler(std::string homePath);
    bool handleRequest(std::string url, HttpArguments arguments, std::vector<char> &response);

};

#endif
