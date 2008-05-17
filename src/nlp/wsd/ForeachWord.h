/*
 * ForeachWord.h
 *
 * Implements a collection of iterators for running over the multiple
 * parses of a sentence, the multiple word-instances of a parse, and so
 * on. The goal here is that these iterators hide the structural detail
 * of the opencog representation of sentences, pares, and so on. Thus,
 * if (when?) the opencog representation changes, then only this file 
 * needs to be adjusted, instead of the broad sweep of algorithms.
 *
 * Copyright (c) 2008 Linas Vepstas <linas@linas.org>
 */

#ifndef OPENCOG_FOREACH_WORD_H
#define OPENCOG_FOREACH_WORD_H

#include <Atom.h>
#include <ForeachChaseLink.h>
#include <FollowLink.h>
#include <Node.h>

namespace opencog {

/**
 * Call the callback for each parse in a sentence.  The argument handle
 * is presumed to identify  a SentenceNode, which is linked to parses 
 * via a ParseLink:
 * 
 *    <ParseLink>
 *      <ConceptNode name="parse_2" strength=0.8 confidence=0.5/>
 *      <SentenceNode name="sentence_22" />
 *    </ParseLink>
 */
template<class T>
inline void foreach_parse(Handle h, bool (T::*cb)(Handle), T *data)
{
	ForeachChaseLink<T> chase;
	chase.backtrack_binary_link(h, PARSE_LINK, cb, data);
}

/**
 * Call the callback for every word-instance in a parse. The argument
 * handle is presumed to identify a specific parse. Each word-instance
 * in the parse is linked to it via a ParseInstanceLink:
 *
 *    <ParseInstanceLink>
 *       <ConceptNode name="bark_169" />
 *       <ConceptNode name="parse_3" />
 *    </ParseInstanceLink>
 */
template <class T>
inline void foreach_word_instance(Handle h, bool (T::*cb)(Handle), T *data)
{
	ForeachChaseLink<T> chase;
	chase.backtrack_binary_link(h, PARSE_INSTANCE_LINK, cb, data);
}

/**
 * Given a dictionary word, call the callback for each word sense
 * associated with that dictionary word, for all parts-of-speech.
 * The argument is presumed to point at a specific dictionary word.
 *
 * Each dictionary-word is assumed to be linked to word senses via
 *
 *    <WordSenseLink>
 *       <WordNode name="bark" />
 *       <ConceptNode name="bark_sense_23" />
 *    </WordSenseLink>
 *  
 */
template <class T>
inline void foreach_dict_word_sense(Handle h, bool (T::*cb)(Handle), T *data)
{
	ForeachChaseLink<T> chase;
	chase.follow_binary_link(h, WORD_SENSE_LINK, cb, data);
}

/**
 * Given a dictionary word, call the callback for each word sense
 * associated with that dictionary word, for the indicated parts-of-speech.
 * The argument is presumed to point at a specific dictionary word.
 *
 * Each dictionary-word is assumed to be linked to word senses via
 *
 *    <WordSenseLink>
 *       <WordNode name="bark" />
 *       <ConceptNode name="bark_sense_23" />
 *    </WordSenseLink>
 *  
 * Each word-sense is assumed to be linked to a part-of-speech via
 *
 *    <PartOfSpeechLink>
 *       <ConceptNode name="bark_sense_23" />
 *       <ConceptNode name="noun" />
 *    </PartOfSpeechLink>
 *
 */
// The PrivateUseOnlyPOSFilter class should be local scope to 
// foreach_dict_word_sense_pos() only, but C++ doesn't allow this. :-(
// This class is not for general, public use!
template <typename T>
class PrivateUseOnlyPOSFilter  
{
	public:
		bool (T::*user_cb)(Handle);
		T *user_data;
		const std::string *desired_pos;
		bool pos_filter(Handle h)
		{
			Atom *word_sense = TLB::getAtom(h);

			// Find the part-of-speech for this word-sense.
			FollowLink fl;
			Atom *a = fl.follow_binary_link(word_sense, PART_OF_SPEECH_LINK);
			Node *n = dynamic_cast<Node *>(a);
			std::string sense_pos = n->getName();

			// If there's no POS match, skip this sense.
			if (desired_pos->compare(sense_pos)) return false;

			// If we are here, there's a match, so call the user callback.
			return (user_data->*user_cb)(h);
		}
};

template <typename T>
inline void foreach_dict_word_sense_pos(Handle h, const std::string &pos,
                                        bool (T::*cb)(Handle), T *data)
{
	PrivateUseOnlyPOSFilter<T> pf;
	pf.user_cb = cb;
	pf.user_data = data;
	pf.desired_pos = &pos;
	ForeachChaseLink<PrivateUseOnlyPOSFilter<T> > chase;
	chase.follow_binary_link(h, WORD_SENSE_LINK,
	                         &PrivateUseOnlyPOSFilter<T>::pos_filter, &pf);
}

/**
 * Return the part-of-speech for the indicated word-instance.
 * @handle:  handle of a word-instance node.
 *
 * Each word-instance is assumed to be linked to a part-of-speech via
 *
 *    <PartOfSpeechLink>
 *       <ConceptNode name="bark_169" />
 *       <DefinedLinguisticConceptNode name="#noun" />
 *    </PartOfSpeechLink>
 */
inline const std::string& get_pos_of_word_instance(Handle h)
{
	Atom * word_instance = TLB::getAtom(h);

	// Find the part-of-speech for this word instance.
	FollowLink fl;
	Atom *inst_pos = fl.follow_binary_link(word_instance, PART_OF_SPEECH_LINK);
	Node *n = dynamic_cast<Node *>(inst_pos);
	return n->getName();
}

/**
 * Return the dictionary-word correspondng to a given word-instance.
 *
 * Each word-instance is assumed to be link to a single WordNode via 
 * a ReferenceLink:
 *
 *    <ReferenceLink>
 *      <ConceptNode name="bark_169" />
 *      <WordNode name="bark">
 *    </ReferenceLink>
 */
inline Handle get_dict_word_of_word_instance(Handle h)
{
	Atom *word_instance = TLB::getAtom(h);
	FollowLink fl;
	Atom *dict_word = fl.follow_binary_link(word_instance, REFERENCE_LINK);
	return TLB::getHandle(dict_word);
}


/**
 * For each word-instance, loop over all syntactic relationships
 * (i.e. _subj, _obj, _nn, _amod, and so on). For each relationship,
 * call the indicated callback. The callback is passed the relation 
 * name, and the two members of the relation. 
 *
 * It is assumed that the relex relationships are structured as follows:
 *
 *    "The outfielder caught the ball."
 *    <!-- _subj (<<catch>>, <<outfielder>>) -->
 *    <EvaluationLink>
 *       <DefinedLinguisticRelationshipNode name="_subj"/>
 *       <ListLink>
 *          <ConceptNode name="catch_instance_23"/>
 *          <ConceptNode name="outfielder_instance_48"/>
 *       </ListLink>
 *    </EvaluationLink>
 *
 * It is assumed that the passed handle indicates the first word
 * instance in the relationship.
 */
// The PrivateUseOnlyRelationFinder class should be local scope to 
// foreach_relex_relation() only, but C++ doesn't allow this. :-(
// This class is not for general, public use!
template <typename T>
class PrivateUseOnlyRelexRelationFinder
{
	private:
		Atom *listlink;
		bool look_for_eval_link(Handle h)
		{
			Atom *a = TLB::getAtom(h);
			if (a->getType() != EVALUATION_LINK) return false;

			// If we are here, lets see if the first node is a ling rel.
			Link *l = dynamic_cast<Link *>(a);
			if (l == NULL) return false;

			a = l->getOutgoingAtom(0);
			Node *n = dynamic_cast<Node *>(a);
			if (n == NULL) return false;
			if (n->getType() != DEFINED_LINGUISTIC_RELATIONSHIP_NODE) return false;

			// OK, we've found a relationship. Get the second member of
			// the list link, and call the suer callback with it.
			const std::string &relname = n->getName();

			l = dynamic_cast<Link *>(listlink);
			const std::vector<Handle> outset = l->getOutgoingSet();

			(user_data->*user_cb)(relname, outset[0], outset[1]);
			return false;
		}
		
	public:
		bool (T::*user_cb)(const std::string &, Handle, Handle);
		T *user_data;

		bool look_for_list_link(Handle h)
		{
			Atom *a = TLB::getAtom(h);
			if (a->getType() != LIST_LINK) return false;
			listlink = a;

			// If we are here, lets see if the list link is in eval link.
			foreach_incoming_handle(h, &PrivateUseOnlyRelexRelationFinder::look_for_eval_link, this);
			return false;
		}
};

template <typename T>
inline void 
foreach_relex_relation(Handle h, 
                       bool (T::*cb)(const std::string &, Handle, Handle), T *data)
{
	PrivateUseOnlyRelexRelationFinder<T> rrf;
	rrf.user_cb = cb;
	rrf.user_data = data;
	foreach_incoming_handle(h, &PrivateUseOnlyRelexRelationFinder<T>::look_for_list_link, &rrf);
}

}

#endif /* OPENCOG_FOREACH_WORD_H */

