/*
 * SchemeEval.c
 *
 * Simple scheme evaluator
 * Copyright (c) 2008 Linas Vepstas <linas@linas.org>
 */

#ifdef HAVE_GUILE

#include "SchemeEval.h"

#include <libguile.h>
#include <libguile/backtrace.h>
#include <libguile/debug.h>
#include <libguile/lang.h>

#include "platform.h"
#include "SchemeSmob.h"

using namespace opencog;

bool SchemeEval::is_inited = false;

SchemeEval::SchemeEval(void)
{
	if (!is_inited)
	{
		is_inited = true;
		scm_init_guile();
		// DO NOT call scm_init_debug(), this will mess up debugging!
		// scm_init_debug();
		// scm_init_backtrace();
		// scm_init_strports(); // is this really needed ?
		SchemeSmob::init();
	}

	outport = scm_open_output_string();
	scm_set_current_output_port(outport);
}

/* ============================================================== */

std::string SchemeEval::prt(SCM node)
{
	if (SCM_SMOB_PREDICATE(SchemeSmob::cog_handle_tag, node))
	{
		return SchemeSmob::handle_to_string(node);
	}

	else if (SCM_SMOB_PREDICATE(SchemeSmob::cog_misc_tag, node))
	{
		return SchemeSmob::misc_to_string(node);
	}

	else if (scm_is_eq(node, SCM_UNSPECIFIED))
	{
		return "";
	}
#if CUSTOM_PRINTING_WHY_DO_WE_HAVE_THIS_DELETE_ME
	else if (scm_is_pair(node))
	{
		std::string str = "(";
      SCM node_list = node;
		const char * sp = "";
      do
      {
			str += sp;
			sp = " ";
         node = SCM_CAR (node_list);
         str += prt (node);
         node_list = SCM_CDR (node_list);
      }
      while (scm_is_pair(node_list));

		// Print the rest -- the CDR part
		if (!scm_is_null(node_list)) 
		{
			str += " . ";
			str += prt (node_list);
		}
		str += ")";
		return str;
   }
	else if (scm_is_true(scm_symbol_p(node))) 
	{
		node = scm_symbol_to_string(node);
		char * str = scm_to_locale_string(node);
		// std::string rv = "'";  // print the symbol escape
		std::string rv = "";      // err .. don't print it
		rv += str;
		free(str);
		return rv;
	}
	else if (scm_is_true(scm_string_p(node))) 
	{
		char * str = scm_to_locale_string(node);
		std::string rv = "\"";
		rv += str;
		rv += "\"";
		free(str);
		return rv;
	}
	else if (scm_is_number(node)) 
	{
		#define NUMBUFSZ 60
		char buff[NUMBUFSZ];
		if (scm_is_signed_integer(node, INT_MIN, INT_MAX))
		{
			snprintf (buff, NUMBUFSZ, "%ld", (long) scm_to_long(node));
		}
		else if (scm_is_unsigned_integer(node, 0, UINT_MAX))
		{
			snprintf (buff, NUMBUFSZ, "%lu", (unsigned long) scm_to_ulong(node));
		}
		else if (scm_is_real(node))
		{
			snprintf (buff, NUMBUFSZ, "%g", scm_to_double(node));
		}
		else if (scm_is_complex(node))
		{
			snprintf (buff, NUMBUFSZ, "%g +i %g", 
				scm_c_real_part(node),
				scm_c_imag_part(node));
		}
		else if (scm_is_rational(node))
		{
			std::string rv;
			rv = prt(scm_numerator(node));
			rv += "/";
			rv += prt(scm_denominator(node));
			return rv;
		}
		return buff;
	}
	else if (scm_is_true(scm_char_p(node))) 
	{
		std::string rv;
		rv = (char) scm_to_char(node);
		return rv;
	}
	else if (scm_is_true(scm_boolean_p(node))) 
	{
		if (scm_to_bool(node)) return "#t";
		return "#f";
	}
	else if (SCM_NULL_OR_NIL_P(node)) 
	{
		// scm_is_null(x) is true when x is SCM_EOL
		// SCM_NILP(x) is true when x is SCM_ELISP_NIL
		return "()";
	}
	else if (scm_is_eq(node, SCM_UNDEFINED))
	{
		return "undefined";
	}
	else if (scm_is_eq(node, SCM_EOF_VAL))
	{
		return "eof";
	}
#endif
	else
	{
		// Let SCM display do the rest of the work.
		SCM port = scm_open_output_string();
		scm_display (node, port);
		SCM rc = scm_get_output_string(port);
		char * str = scm_to_locale_string(rc);
		std::string rv = str;
		free(str);
		scm_close_port(port);
		return rv;
	}

	return "";
}

/* ============================================================== */

SCM SchemeEval::preunwind_handler_wrapper (void *data, SCM tag, SCM throw_args)
{
	SchemeEval *ss = (SchemeEval *)data;
	return ss->preunwind_handler(tag, throw_args);
	return SCM_EOL;
}

SCM SchemeEval::catch_handler_wrapper (void *data, SCM tag, SCM throw_args)
{
	SchemeEval *ss = (SchemeEval *)data;
	return ss->catch_handler(tag, throw_args);
}

SCM SchemeEval::preunwind_handler (SCM tag, SCM throw_args)
{
	// We can only record the stack before it is unwound. 
	// The normal catch handler body runs only *after* the stack
	// has been unwound.
	captured_stack = scm_make_stack (SCM_BOOL_T, SCM_EOL);
	return SCM_EOL;
}

SCM SchemeEval::catch_handler (SCM tag, SCM throw_args)
{
	// Check for read error. If a read error, then wait for user to correct it.
	SCM re = scm_symbol_to_string(tag);
	char * restr = scm_to_locale_string(re);
	pending_input = false;
	if (0 == strcmp(restr, "read-error"))
	{
		pending_input = true;
		free(restr);
		return SCM_EOL;
	}

	// If its not a read error, then its a regular error; report it.
	caught_error = true;

	/* get string port into which we write the error message and stack. */
	error_string_port = scm_open_output_string();
	SCM port = error_string_port;

	if (scm_is_true(scm_list_p(throw_args)) && (scm_ilength(throw_args) >= 1))
	{
		long nargs = scm_ilength(throw_args);
		SCM subr    = SCM_CAR (throw_args);
		SCM message = SCM_EOL;
		if (nargs >= 2)
			message = SCM_CADR (throw_args);
		SCM parts   = SCM_EOL;
		if (nargs >= 3)
			parts   = SCM_CADDR (throw_args);
		SCM rest    = SCM_EOL;
		if (nargs >= 4)
			rest    = SCM_CADDDR (throw_args);

		if (scm_is_true (captured_stack))
		{
			SCM highlights;

			if (scm_is_eq (tag, scm_arg_type_key) ||
			    scm_is_eq (tag, scm_out_of_range_key))
				highlights = rest;
			else
				highlights = SCM_EOL;

			scm_puts ("Backtrace:\n", port);
			scm_display_backtrace_with_highlights (captured_stack, port,
			      SCM_BOOL_F, SCM_BOOL_F, highlights);
			scm_newline (port);
		}
		scm_display_error (captured_stack, port, subr, message, parts, rest);
	}
	else
	{
		scm_puts ("ERROR: thow args are unexpectedly short!\n", port);
	}
	scm_puts("ABORT: ", port);
	scm_puts(restr, port);
	free(restr);

	return SCM_BOOL_F;
}

/* ============================================================== */

/**
 * Evaluate the expression
 */
std::string SchemeEval::eval(const std::string &expr)
{
	input_line += expr;

	caught_error = false;
	pending_input = false;
#if 0
	SCM rc = scm_internal_catch (SCM_BOOL_T,
	            (scm_t_catch_body) scm_c_eval_string, (void *) input_line.c_str(),
	            SchemeEval::catch_handler_wrapper, this);
#endif
	captured_stack = SCM_BOOL_F;
	SCM rc = scm_c_catch (SCM_BOOL_T,
	            (scm_t_catch_body) scm_c_eval_string, (void *) input_line.c_str(),
	            SchemeEval::catch_handler_wrapper, this,
	            SchemeEval::preunwind_handler_wrapper, this);

	if (pending_input)
	{
		return "";
	}
	pending_input = false;
	input_line = "";

	if (caught_error)
	{
		std::string rv;
		rc = scm_get_output_string(error_string_port);
		char * str = scm_to_locale_string(rc);
		rv = str;
		free(str);
		scm_close_port(error_string_port);

		scm_truncate_file(outport, scm_from_uint16(0));

		return rv;
	}
	else
	{
		std::string rv;
		// First, we get the contents of the output port,
		// and pass that on.
		SCM out = scm_get_output_string(outport);
		char * str = scm_to_locale_string(out);
		rv = str;
		free(str);
		scm_truncate_file(outport, scm_from_uint16(0));

		// Next, we append the "interpreter" output
		rv += prt(rc);
		rv += "\n";

		return rv;
	}
	return "#<Error: Unreachable statement reached>";
}

#endif
/* ===================== END OF FILE ============================ */
