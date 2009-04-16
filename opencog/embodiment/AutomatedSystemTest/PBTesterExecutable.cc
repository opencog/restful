/*
 * opencog/embodiment/AutomatedSystemTest/PBTesterExecutable.cc
 *
 * Copyright (C) 2002-2009 Novamente LLC
 * All Rights Reserved
 * Author(s): Welter Luigi
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


/**
 * PBTester.cc
 * This program performs automated system tests on PB-side of Petaverse code.
 * It simulates the PVP Proxy by reading from a file a sequence of messages to be sent and
 * received to/from Petaverse ROUTER.
 *
 * Author: Welter Luigi
 */


#include "PBTester.h"
#include <exception>
#include <unistd.h>
#include <opencog/util/files.h>
#include "GoldStdReaderAgent.h"
#include "TestConfig.h"

using namespace AutomatedSystemTest;
using namespace opencog;

int main(int argc, char *argv[])
{

    // Open/read the data file passed as argument
    if (argc < 2) {
        printf("Wrong number of arguments:\nExpected: %s <Gold Standard Filename>\n", argv[0]);
    }
    const char* filename = argv[1];

    config(TestConfig::testCreateInstance, true);

    if (fileExists(config().get("CONFIG_FILE").c_str())) {
        config().load(config().get("CONFIG_FILE").c_str());
    }

    if (fileExists(config().get("CONFIG_FILE").c_str())) {
        config().load(config().get("CONFIG_FILE").c_str());
    }

    server(PBTester::createInstance);
    PBTester& pbTester = static_cast<PBTester&>(server());
    pbTester.init(config().get("PROXY_ID"), config().get("PROXY_IP"), config().get_int("PROXY_PORT"));

    Factory<GoldStdReaderAgent, Agent> goldStdReaderAgentFactory;

    pbTester.registerAgent(GoldStdReaderAgent::info().id, &goldStdReaderAgentFactory);
    GoldStdReaderAgent* goldStdReaderAgent = static_cast<GoldStdReaderAgent*>(
                pbTester.createAgent(GoldStdReaderAgent::info().id, false));
    goldStdReaderAgent->init(filename);
    pbTester.startAgent(goldStdReaderAgent);

    try {
        pbTester.serverLoop();
    } catch (std::bad_alloc) {
        logger().log(Logger::ERROR, "PBTesterExec - PBTester raised a bad_alloc exception.");

    } catch (...) {
        logger().log(Logger::ERROR,
                     "PBTesterExec - An exceptional situation occured. Check log for information.");
    }

    return 0;
}

