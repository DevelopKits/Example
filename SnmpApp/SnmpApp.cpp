// SnmpApp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

int main(int argc, char ** argv)
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	netsnmp_variable_list *vars;
	int status;
	int count = 1;

	init_snmp("snmpdemoapp");


	snmp_sess_init(&session);                
	session.peername = strdup("test.net-snmp.org");

	session.version = SNMP_VERSION_1;

	/* set the SNMPv1 community name used for authentication */
	session.community = "public";
	session.community_len = strlen(session.community);


	SOCK_STARTUP;
	ss = snmp_open(&session);                   

	if (!ss) {
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	pdu = snmp_pdu_create(SNMP_MSG_GET);
	anOID_len = MAX_OID_LEN;
	if (!snmp_parse_oid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len)) 
	{
		snmp_perror(".1.3.6.1.2.1.1.1.0");
		SOCK_CLEANUP;
		exit(1);
	}
#if 0
	/*
	*  These are alternatives to the 'snmp_parse_oid' call above,
	*    e.g. specifying the OID by name rather than numerically.
	*/
	read_objid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len);
	get_node("sysDescr.0", anOID, &anOID_len);
	read_objid("system.sysDescr.0", anOID, &anOID_len);
#endif

	snmp_add_null_var(pdu, anOID, anOID_len);

	status = snmp_synch_response(ss, pdu, &response);


	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) 
	{

		for (vars = response->variables; vars; vars = vars->next_variable)
			print_variable(vars->name, vars->name_length, vars);

		for (vars = response->variables; vars; vars = vars->next_variable) 
		{
			if (vars->type == ASN_OCTET_STR)
			{
				char *sp = (char *)malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			}
			else
				printf("value #%d is NOT a string! Ack!\n", count++);
		}
	}
	else 
	{

		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n",
			snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n",
			session.peername);
		else
			snmp_sess_perror("snmpdemoapp", ss);

	}

	if (response)
		snmp_free_pdu(response);
	snmp_close(ss);

	SOCK_CLEANUP;
	return (0);
} 
	