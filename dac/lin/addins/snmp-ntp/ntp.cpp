
#define USING_UTIL_FUNCS_MODULE 1
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/util_funcs.h>

#include "ntp.h"
#include "ntpq.h"

#include <exception>

using namespace std;

/* 
 * ntp_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid ntp_variables_oid[] = { 1,3,6,1,4,1,2021,13,5000,1,1,2,1,1 };

/* 
 * variable4 ntp_variables:
 *   this variable defines function callbacks and type return information 
 *   for the  mib section 
 */

struct variable4 ntp_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */
#define NTPSTATUS		1
{NTPSTATUS,	ASN_INTEGER,	RONLY ,  var_ntp, 1,  { 1 }},
#define NTPOFFSET		2
{NTPOFFSET,	ASN_OCTET_STR,	RONLY ,  var_ntp, 1,  { 2 }},
#define NTPSRC			3
{NTPSRC,	ASN_OCTET_STR,	RONLY ,  var_ntp, 1,  { 3 }},
};
/*    (L = length of the oidsuffix) */


/** Initializes the ntp module */
void init_ntp( void ) {

	DEBUGMSGTL(("ntp", "Initializing\n"));

	try {
	}
	catch(exception& ex) {
		snmp_log(LOG_ERR, "ERROR creating ntp session: %s\n", ex.what());
 		return;
	}

	/* register ourselves with the agent to handle our mib tree */
	REGISTER_MIB("ntp", ntp_variables, variable4, ntp_variables_oid);

	/* place any other initialization junk you need here */
}

/*
 * var_ntp():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
unsigned char * var_ntp	(
	struct variable		*  vp, 
	oid			*  name, 
	size_t			*  length, 
	int			   exact, 
	size_t			*  var_len, 
	WriteMethod		** write_method
			)
{
	try {
		/* variables we may use later */
		static long long_ret;
		static float float_ret;
		static char string[SPRINT_MAX_LEN];

		if( header_generic(vp,name,length,exact,var_len,write_method) == MATCH_FAILED )
			return NULL;

		/*
		* this is where we do the value assignments for the mib results.
		*/
		switch(vp->magic) {
		case NTPSTATUS:
			long_ret = get_status();
			return (unsigned char *) &long_ret;
		case NTPOFFSET:
			strncpy(string, get_offset().c_str(), SPRINT_MAX_LEN-1);
			string[SPRINT_MAX_LEN-1] = 0;
			*var_len = strlen(string);
			return (unsigned char *) &string;
		case NTPSRC:
			strncpy(string, get_src().c_str(), SPRINT_MAX_LEN-1);
			string[SPRINT_MAX_LEN-1] = 0;
			*var_len = strlen(string);
			return (unsigned char *) &string;
		default:
			ERROR_MSG("");
		}
		return NULL;
	}
	catch(exception& ex) {
		snmp_log(LOG_ERR, "ERROR getting params: %s\n", ex.what());
		return 0;
	}
}
