/*
 * opencog/embodiment/Control/Procedure/RunningBuiltInProcedure.cc
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
#include "RunningBuiltInProcedure.h"

using namespace Procedure;
using namespace PerceptionActionInterface;

RunningBuiltInProcedure::RunningBuiltInProcedure(const PAI& _pai, const BuiltInProcedure& _p, const std::vector<combo::vertex>& _arguments) : pai(_pai), p(_p), arguments(_arguments)
{
    finished = false;
    failed = false;
    result = combo::id::null_vertex; // TODO: perhaps there is a "undefined result" constant or something like that...
}
RunningBuiltInProcedure::~RunningBuiltInProcedure() {}

void RunningBuiltInProcedure::run()
{
    if (finished) return; // must run only once.
    try {
        result = p.execute(arguments);
    } catch (...) {
        failed = true;
    }
    finished = true;
}

bool RunningBuiltInProcedure::isFinished() const
{
    if (!finished) return false;
    if (!p.isPetAction()) return true;
    const ActionPlanID* planId = boost::get<ActionPlanID>(&result);
    if (!planId) return true;
    return pai.isPlanFinished(*planId);
}

bool RunningBuiltInProcedure::isFailed() const
{
    if (failed) return true;
    if (p.isPetAction() && finished) {
        const ActionPlanID* planId = boost::get<ActionPlanID>(&result);
        if (!planId) return true;
        return pai.hasPlanFailed(*planId);
    } else {
        return false;
    }
}

combo::vertex RunningBuiltInProcedure::getResult() const
{
    return result;
}

