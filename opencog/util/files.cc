/*
 * opencog/util/files.cc
 *
 * Copyright (C) 2002-2007 Novamente LLC
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <fstream>
#include <iostream>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>

#include "files.h"

#ifdef WIN32
#include <direct.h>
#define  mkdir _mkdir
#endif

#define USER_FLAG "$USER"

bool fileExists(const char* filename)
{
    std::fstream dumpFile(filename, std::ios::in);
    dumpFile.close();

    if (dumpFile.fail()) {
        dumpFile.clear(std::ios_base::failbit);
        return false;
    }
    return true;
}

void expandPath(std::string& path)
{

    size_t user_index = path.find(USER_FLAG, 0);
    if (user_index != std::string::npos) {
        const char* username = getenv("LOGNAME");

        if (username == NULL) {
            username = "unknown_user";
        }
        path.replace(user_index, strlen(USER_FLAG), username);
    }

    return;
}

bool createDirectory(const char* directory)
{

#ifdef WIN32
    if (mkdir(directory) == 0 || errno == EEXIST) {
#else
    if (mkdir(directory, S_IRWXU | S_IRWXG | S_IRWXO) == 0 || errno == EEXIST) {
#endif
        return true;
    }
    return false;
}

bool appendFileContent(const char* filename, std::string &s) {
    std::ifstream in(filename);
    if (!in.is_open())
        return false;

    char c;
    std::string str;
    while (in.get(c))
        str += c;

    if (!in.eof())
        return false;

    s = str;
    return true;
}

