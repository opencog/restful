/*
 * opencog/server/SleepRequest.cc
 *
 * Copyright (C) 2008 by Singularity Institute for Artificial Intelligence
 * All Rights Reserved
 *
 * Written by Welter Luigi <welter@vettalabs.com>
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

#include "SleepRequest.h"

#include <sstream>

using namespace opencog;

SleepRequest::SleepRequest()
{
}

SleepRequest::~SleepRequest()
{
}

bool SleepRequest::execute()
{
    if (_parameters.size() > 0) {
        int seconds = atoi(_parameters.begin()->c_str()); 
        usleep(seconds*1000000);
    } else {
        // default
        usleep(5000000);
    }
    return true;
}
