		<!-- %CODE%DEV%PAR1 ��������� ����� ����� %NUM ����. (r) -->
		<snmp		id	= "%CODE%DEV%PAR1"
				ptype	= "value"
				vtype   = "short"
				rexp    = ".*\s+"
				addr    = ".1.3.6.1.2.1.2.2.1.8.%OID"
				sdep    = "%CODE%DEV001"
				update  = "5000" >

			<rule key="1" value="2" />
			<rule key="2" value="0" />
			<rule key="3" value="4" />
		</snmp>

		<!-- %CODE%DEV%PAR2 ��������� ����� ����� %NUM ����. (r) -->
		<calc	 	id     = "%CODE%DEV%PAR2" 
				ptype  = "value"
				vtype  = "string"
				math   = ",%CODE%DEV%PAR1">

			<rule key="0" value="���" />
			<rule key="2" value="����"  />
			<rule key="4" value="�� ��������"  />
		</calc>

