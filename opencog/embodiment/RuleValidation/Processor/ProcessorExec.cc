/*
 * opencog/embodiment/RuleValidation/Processor/ProcessorExec.cc
 *
 * Copyright (C) 2007-2008 TO_COMPLETE
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
#include "RuleProcessor.h"
#include "util/files.h"
#include "util/exceptions.h"
#include <SystemParameters.h>

int main(int argc, char * argv[]) {

    if(argc != 3){
        fprintf(stdout, "processor <scenario-file> <type: pet or humanoid>\n");
        return (1);
    }

    Control::SystemParameters parameters;
    if(fileExists(parameters.get("CONFIG_FILE").c_str())){
    	parameters.loadFromFile(parameters.get("CONFIG_FILE"));
    }

    if((strcmp(argv[2], "pet") != 0) &&
       (strcmp(argv[2], "humanoid") != 0)){
        fprintf(stdout, "processor <scenario-file> <type: pet or humanoid>. Got '%s'.\n", argv[2]);
        return (1);
    }

    Processor::RuleProcessor rp(parameters,std::string(argv[2]));

    try{
        rp.evaluateRules(std::string(argv[1]));
    } catch(...){
        fprintf(stdout, "An error has occured while evaluating rules. Check log.\n");
        return (1);
    }
    return (0);
}
