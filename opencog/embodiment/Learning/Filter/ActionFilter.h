/*
 * opencog/embodiment/Learning/Filter/ActionFilter.h
 *
 * Copyright (C) 2007-2008 Nil Geisweiller
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
#ifndef ACTIONFILTER_H_
#define ACTIONFILTER_H_

#include "util/RandGen.h"

#include "comboreduct/combo/vertex.h"

#include "WorldProvider.h"
#include "BDRetriever.h"
#include "BehaviorCategory.h"
#include "CompositeBehaviorDescription.h"
#include "ElementaryBehaviorDescription.h"
#include "WorldWrapperUtil.h"
#include "PetComboVocabulary.h"

namespace Filter
{

using namespace combo;
using namespace behavior;

typedef combo_tree::iterator pre_it;
typedef combo_tree::sibling_iterator sib_it;

class ActionFilter
{
public:

    typedef std::set<combo_tree, opencog::size_tree_order<vertex> > combo_tree_ns_set;
    typedef combo_tree_ns_set::iterator combo_tree_ns_set_it;
    typedef combo_tree_ns_set::const_iterator combo_tree_ns_set_const_it;

    typedef std::set<vertex> object_set;
    typedef object_set::iterator object_set_it;
    typedef object_set::const_iterator object_set_const_it;

    //constructor, desctructor
    //arity denotes the number of input arguments
    //of the combo program to learn
    ActionFilter(const std::string& pet_id,
                 const std::string& owner_id,
                 const WorldProvider& wp,
                 const definite_object_set& dos,
                 const indefinite_object_set& idos,
                 const builtin_action_set& bas,
                 arity_t arity,
                 bool type_check,
                 opencog::RandGen& rng);
    ~ActionFilter();

    //insert all action sequences and subsequences of size <=n
    //compatible with a given BehaviorCategory in actSeq_set.
    //If max_size == -1, then all subsequences are returned.
    //bc will not be modified by that method (const has been omitted because
    //access methods of BehaviorCategory transparently modifies it)
    void insertActionSubseqs(combo_tree_ns_set& actSubseq_set,
                             const BehaviorCategory& bc,
                             const argument_list_list& all,
                             int max_size = -1) const;

    //like above but apply on a CompositeBehaviorDescription instead of
    //BehaviorCategory
    //If max_size == -1, then all subsequences are returned.
    void insertActionSubseqs(combo_tree_ns_set& actSubseq_set,
                             const CompositeBehaviorDescription& cbd,
                             const argument_list& al,
                             int max_size = -1) const;

private:
    const std::string& _self_id;
    const std::string& _owner_id;
    const WorldProvider& _wp;
    const definite_object_set& _dos;
    const indefinite_object_set& _ios;
    const builtin_action_set& _bas;
    arity_t _arity;
    bool _type_check; //type check the generated actions

    opencog::RandGen& _rng;

    //generates the complete action sequence set of a given
    //CompositeBehaviorDescription
    //WARNING :  to work properly it is assumed that actSeq_set is empty
    void generateCompleteActionSeqs(combo_tree_ns_set& actSeq_set,
                                    const CompositeBehaviorDescription& cbd,
                                    const argument_list& al) const;

    //completes a prefix set of action sequences by all possible suffixes
    //sequences compatible with a given CompositeBehaviorDescription
    //considered from a given index
    void completeActionPrefixes(combo_tree_ns_set& actPrefix_set,
                                const CompositeBehaviorDescription& cbd,
                                const argument_list& al,
                                unsigned index) const;

    //insert in act_set all possible actions matching a given
    //elementary BD under the form of behaved atom structure (ebd)
    //and a handle pointing to a spacemap of that time (smh)
    void generatePossibleActions(combo_tree_ns_set& act_set,
                                 Handle ebd,
                                 const argument_list& al,
                                 Handle smh,
                                 unsigned long time) const;


    //insert in opras_set all possible action operands matching a given
    //operand vertex list 'ovl'
    //and a handle pointing to a spacemap of that time (smh)
    //WARNING :  to work properly it is assumed that the set is empty
    void generatePossibleOperands(std::set< std::vector<vertex> >& opras_set,
                                  const std::vector<vertex>& ovl,
                                  const argument_list& al,
                                  Handle smh,
                                  unsigned long time) const;

    //assume a prefix of operands and complete it
    //that is each vector of prefix_opras is completed by one more vertex
    //according to ovl
    void completePrefixOperands(std::set< std::vector<vertex> >& prefix_opras,
                                unsigned index,
                                const std::vector<vertex>& ovl,
                                const argument_list& al,
                                Handle smh,
                                unsigned long time) const;

    //like generatePossibleOperands
    //but works on the level of one single operand
    //more specifically operand will be a constant, like definite_objec,
    //contin or boolean and opras will contain all vertex that can fit that
    //constant, that is the constant itself + perhaps a variable that fits
    //or an indefinite_object and so forth
    void generatePossibleOperands(std::set<vertex>& opras,
                                  const vertex& operand,
                                  const argument_list& al,
                                  Handle smh,
                                  unsigned long time) const;

};

}

#endif
