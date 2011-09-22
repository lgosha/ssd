// MyAgent.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <snmp.h>
#include <malloc.h>

#include <QtCore/QProcess>

#define OID_SIZEOF( Oid )      ( sizeof Oid / sizeof(UINT) )

// template MIB entry
struct MIB_ENTRY
{
	AsnObjectIdentifier asnOid;
	void *              pStorageValue;
	CHAR *              szStorageName;
	BYTE                chType;
	UINT                unAccess;
	MIB_ENTRY*			pMibNext;
};

// this is the our branch starting point (clled prefix)
UINT g_unMyOIDPrefix[]	= {1, 3, 6, 1, 4, 1, 9999};

AsnObjectIdentifier MIB_OidPrefix = { OID_SIZEOF(g_unMyOIDPrefix), g_unMyOIDPrefix};

DWORD g_dwUpdateTime = 0;

void updateValues();

int GetRequest(SnmpVarBind *pVarBind);
int GetNextRequest(SnmpVarBind *pVarBind);
int SetRequest(SnmpVarBind *pVarBind);

UINT GetStoreVar(MIB_ENTRY* pMIB, AsnAny *pasnValue);
UINT SetStoreVar(MIB_ENTRY* pMIB, AsnAny asnValue);

/////////MIB Table ////////////////////////////////////////////////
UINT g_unAboutOid[] = {0,0,1};
UINT g_unNameOid[] = {0,0,2};
UINT g_unAgeOid[] = {0,0,3};

char *g_szAbout = NULL;
char *g_szName = NULL;
AsnInteger g_asnIntAge = 0;

/// This is the MIB table and its related variable store
//    here evry thing is hard-coded to demonstration perpose
//    Actualy it should be loaded from the registry or from some file
MIB_ENTRY g_MyMibTable[] = {
	{	
		{OID_SIZEOF(g_unAboutOid),g_unAboutOid},
		&g_szAbout,
		"Source",
		ASN_OCTETSTRING,
		SNMP_ACCESS_READ_ONLY,
		&g_MyMibTable[1]
	},
	{
		{OID_SIZEOF(g_unNameOid),g_unNameOid},
		&g_szName,
		"Offset",
		ASN_OCTETSTRING,
		SNMP_ACCESS_READ_ONLY,
		&g_MyMibTable[2]
	},
	{
		{OID_SIZEOF(g_unAgeOid),g_unAgeOid},
		&g_asnIntAge,
		"Status",
		ASN_INTEGER,
		SNMP_ACCESS_READ_ONLY,
		NULL
	}
};

UINT g_unMyMibCount = (sizeof(g_MyMibTable) / sizeof(MIB_ENTRY));
///////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HANDLE hModule,DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// When exported funtion will be called during DLL loading and initialization
BOOL SNMP_FUNC_TYPE SnmpExtensionInit(DWORD dwUptimeReference,HANDLE *phSubagentTrapEvent, AsnObjectIdentifier *pFirstSupportedRegion)
{
	*pFirstSupportedRegion = MIB_OidPrefix;

	g_szAbout = (char*)malloc(sizeof(char)*64);
	g_szName = (char*)malloc(sizeof(char)*64);
	
	updateValues();

	return SNMPAPI_NOERROR;
}

void updateValues() {

	if( GetTickCount() - g_dwUpdateTime < 5 ) return;

	int status = 0;
	std::string src = "0.0.0.0";
	std::string offset = "0.000";

	QProcess proc;
	proc.start("ntpq", QStringList() << "-n" << "-p");
	// exec ntpq -p
	if( proc.waitForFinished( 5000 ) && proc.exitCode() == 0 ) {
		QList<QByteArray> lstderr = proc.readAllStandardError().split('\n');
		foreach( QByteArray ba, lstderr ) {
			if( ba.indexOf("error") != -1 ) { status = 1; break; }
		}
		// ntp up
		if( status == 0 ) {
			QList<QByteArray> lstdout = proc.readAllStandardOutput().split('\n');
			QString str = QString();
			foreach( QByteArray ba, lstdout ) {
				if( ba.indexOf('*') != -1 ) { status = 3; str = ba; break; }
			}
			// ntp sync
			if( !str.isNull() ) {
				QStringList lstr = str.split( QRegExp("\\s+") );
				offset = lstr.at( lstr.size() - 2 ).toStdString();
				QString sTmp = lstr.at( 0 );
				src = sTmp.replace('*', "").toStdString();
			}
			else status = 2;
		}
	}

	g_asnIntAge = status;
	strcpy( g_szAbout, src.c_str() );
	strcpy( g_szName, offset.c_str() );

	g_dwUpdateTime = GetTickCount();
}

// this export is to query the MIB table and fields
BOOL SNMP_FUNC_TYPE SnmpExtensionQuery(BYTE bPduType, SnmpVarBindList *pVarBindList, AsnInteger32 *pErrorStatus, AsnInteger32 *pErrorIndex)
{
	int nRet = 0;
	AsnObjectName;
	
	*pErrorStatus = SNMP_ERRORSTATUS_NOERROR;
	*pErrorIndex = 0;

	updateValues();

	for(UINT i=0;i<pVarBindList->len;i++)
	{
		*pErrorStatus = SNMP_ERRORSTATUS_NOERROR;

		// what type of request we are getting?
		switch(bPduType)
		{
		case SNMP_PDU_GET:// // gets the variable value passed variable in pVarBindList
			*pErrorStatus = GetRequest(&pVarBindList->list[i]);
			if(*pErrorStatus != SNMP_ERRORSTATUS_NOERROR)
				*pErrorIndex++;
			break;
		case SNMP_PDU_GETNEXT: // gets the next variable related to the passed variable in pVarBindList
			*pErrorStatus = GetNextRequest(&pVarBindList->list[i]);
			if(*pErrorStatus != SNMP_ERRORSTATUS_NOERROR)
				*pErrorIndex++;
			break;
		case SNMP_PDU_SET: // sets a variable
			*pErrorStatus = SetRequest(&pVarBindList->list[i]);
			if(*pErrorStatus != SNMP_ERRORSTATUS_NOERROR)
				*pErrorIndex++;
			break;
		default:
			*pErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
			*pErrorIndex++;
		};
	}	

	return SNMPAPI_NOERROR;
}
 
// get the value of the variable pased
int GetRequest(SnmpVarBind *pVarBind)
{
	int nRet = SNMP_ERRORSTATUS_NOSUCHNAME;
	AsnObjectIdentifier  asnOidTemp;

	for(UINT i=0;i<g_unMyMibCount;i++)
	{
		SnmpUtilOidCpy(&asnOidTemp, &MIB_OidPrefix);
		SnmpUtilOidAppend(&asnOidTemp, &g_MyMibTable[i].asnOid);
		if(SnmpUtilOidCmp(&asnOidTemp,&pVarBind->name) ==0)
		{
			SnmpUtilOidFree(&asnOidTemp);

			nRet = GetStoreVar(&g_MyMibTable[i],&pVarBind->value);
			break;
		}
		SnmpUtilOidFree(&asnOidTemp);
	}
	return nRet;
}

// get the next variable & value related to the passed variable
int GetNextRequest(SnmpVarBind *pVarBind)
{
	int nRet = SNMP_ERRORSTATUS_NOSUCHNAME;
	int nResult = 0;
	AsnObjectIdentifier  asnOidTemp;

	for(UINT i=0;i<g_unMyMibCount;i++)
	{
		SnmpUtilOidCpy(&asnOidTemp, &MIB_OidPrefix);
		SnmpUtilOidAppend(&asnOidTemp, &g_MyMibTable[i].asnOid);

		nResult = SnmpUtilOidCmp(&asnOidTemp,&pVarBind->name);
		if(nResult >0)
		{
			SnmpUtilOidFree(&pVarBind->name);

			// kind of memcpy
			SnmpUtilOidCpy(&pVarBind->name, &asnOidTemp);

			nRet = GetStoreVar(&g_MyMibTable[i],&pVarBind->value);
			SnmpUtilOidFree(&asnOidTemp);
			nRet = SNMP_ERRORSTATUS_NOERROR;
			break;
		}
		else if(nResult == 0)
		{
			SnmpUtilOidFree(&asnOidTemp);
			if(g_MyMibTable[i].pMibNext == NULL)
				break;
			// king of mem free
			SnmpUtilOidFree(&pVarBind->name);			

			// kind of memcpy :)
			SnmpUtilOidCpy(&pVarBind->name, &MIB_OidPrefix);

			// kind of strcat
			SnmpUtilOidAppend(&pVarBind->name, &g_MyMibTable[i].pMibNext->asnOid);

			nRet = GetStoreVar(g_MyMibTable[i].pMibNext,&pVarBind->value);
			break;
		}
		SnmpUtilOidFree(&asnOidTemp);
	}
	return nRet;
}

// if it is a writable variable passed value will be copied to it
int SetRequest(SnmpVarBind *pVarBind)
{
	int nRet = SNMP_ERRORSTATUS_NOSUCHNAME;
	AsnObjectIdentifier  asnOidTemp;

	for(UINT i=0;i<g_unMyMibCount;i++)
	{
		// kind of memcpy ;), any way you can't use memcpy 
		SnmpUtilOidCpy(&asnOidTemp, &MIB_OidPrefix);
		
		// kind of strcat ;)
		SnmpUtilOidAppend(&asnOidTemp, &g_MyMibTable[i].asnOid);

		if(SnmpUtilOidCmp(&asnOidTemp,&pVarBind->name) ==0)
		{
			// kind of SNMP 'free'
			SnmpUtilOidFree(&asnOidTemp);
			nRet = SetStoreVar(&g_MyMibTable[i],pVarBind->value);
			break;
		}
		SnmpUtilOidFree(&asnOidTemp);
	}
	return nRet;
}

// retrieves the value from the MIB table
//  Imagine AsnAny is like VARIANT structure to populate that you need to go through the types
UINT GetStoreVar(MIB_ENTRY* pMIB, AsnAny *pasnValue)
{
	// check rights is there to access
	if((pMIB->unAccess != SNMP_ACCESS_READ_ONLY)&&(pMIB->unAccess != SNMP_ACCESS_READ_WRITE)&&(pMIB->unAccess != SNMP_ACCESS_READ_CREATE))
		return SNMP_ERRORSTATUS_GENERR;

	// set the type
	pasnValue->asnType = pMIB->chType;
	
	switch(pasnValue->asnType)
	{
	case ASN_INTEGER:
		pasnValue->asnValue.number = *(AsnInteger32*)pMIB->pStorageValue;
		break;
	case ASN_COUNTER32:
	case ASN_GAUGE32:
	case ASN_TIMETICKS:
	case ASN_UNSIGNED32:
		pasnValue->asnValue.unsigned32 = *(AsnUnsigned32*)pMIB->pStorageValue;
		break;
	case ASN_OCTETSTRING:
		pasnValue->asnValue.string.length = strlen(*(LPSTR*)pMIB->pStorageValue);
		pasnValue->asnValue.string.stream =(unsigned char*)SnmpUtilMemAlloc(pasnValue->asnValue.string.length * sizeof(char));

		memcpy(pasnValue->asnValue.string.stream,*(LPSTR*)pMIB->pStorageValue,pasnValue->asnValue.string.length);
		pasnValue->asnValue.string.dynamic = TRUE;
		break;
	case ASN_COUNTER64:
		pasnValue->asnValue.counter64 = *(AsnCounter64*)pMIB->pStorageValue;
		break;
	case ASN_OBJECTIDENTIFIER:
		SnmpUtilOidCpy(&pasnValue->asnValue.object,(AsnObjectIdentifier*)pMIB->pStorageValue);
		break;
	case ASN_IPADDRESS:
		pasnValue->asnValue.address.length = 4;
		pasnValue->asnValue.string.dynamic = TRUE;

		pasnValue->asnValue.address.stream[0] = ((char*)pMIB->pStorageValue)[0];
		pasnValue->asnValue.address.stream[1] = ((char*)pMIB->pStorageValue)[1];
		pasnValue->asnValue.address.stream[2] = ((char*)pMIB->pStorageValue)[2];
		pasnValue->asnValue.address.stream[3] = ((char*)pMIB->pStorageValue)[3];
		break;
	case ASN_OPAQUE:
		AsnSequence;
		break;
	case ASN_BITS:
		break;	
	case ASN_SEQUENCE:
		break;	
	case ASN_NULL:
	default:
		return SNMP_ERRORSTATUS_GENERR;	
	}
	return SNMP_ERRORSTATUS_NOERROR;
}

// to store the value to the MIB table
//  Imagine AsnAny is like VARIANT structure to populate that you need to go through the types
UINT SetStoreVar(MIB_ENTRY* pMIB, AsnAny asnValue)
{
	// check whether rights is there to access
	if((pMIB->unAccess != SNMP_ACCESS_READ_WRITE)&&(pMIB->unAccess != SNMP_ACCESS_READ_CREATE))
		return SNMP_ERRORSTATUS_READONLY;

	// check type matches with the table type
	if(pMIB->chType != asnValue.asnType)
		return SNMP_ERRORSTATUS_BADVALUE;

	switch(asnValue.asnType)
	{
	case ASN_INTEGER:
		*(AsnInteger *)(pMIB->pStorageValue) = asnValue.asnValue.number;
		break;
	case ASN_COUNTER32:
	case ASN_GAUGE32:
	case ASN_TIMETICKS:
	case ASN_UNSIGNED32:
		*(AsnInteger *)(pMIB->pStorageValue) = asnValue.asnValue.unsigned32;
		break;
	case ASN_OCTETSTRING:
		free(*(LPSTR*)(pMIB->pStorageValue));
		*(LPSTR*)(pMIB->pStorageValue) = (LPSTR)malloc((asnValue.asnValue.string.length+1) * sizeof(char));
		memcpy(*(LPSTR*)(pMIB->pStorageValue), asnValue.asnValue.string.stream,asnValue.asnValue.string.length);
		(*(LPSTR*)pMIB->pStorageValue)[asnValue.asnValue.string.length] = '\0';
		break;
	case ASN_COUNTER64:
		*(AsnCounter64*)pMIB->pStorageValue = asnValue.asnValue.counter64;
		break;
	case ASN_OBJECTIDENTIFIER:
		SnmpUtilOidCpy((AsnObjectIdentifier*)pMIB->pStorageValue,&asnValue.asnValue.object);
		break;

	case ASN_IPADDRESS:
		((char*)pMIB->pStorageValue)[0] = asnValue.asnValue.address.stream[0];
		((char*)pMIB->pStorageValue)[1] = asnValue.asnValue.address.stream[1];
		((char*)pMIB->pStorageValue)[2] = asnValue.asnValue.address.stream[2];
		((char*)pMIB->pStorageValue)[3] = asnValue.asnValue.address.stream[3];
		break;
	case ASN_OPAQUE:
		AsnSequence;
		break;
	case ASN_BITS:
		break;	
	case ASN_SEQUENCE:
		break;	
	case ASN_NULL:
		break;
	default:
		return SNMP_ERRORSTATUS_GENERR;	
	}
	return SNMP_ERRORSTATUS_NOERROR;
}

