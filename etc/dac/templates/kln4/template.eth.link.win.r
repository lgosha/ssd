		<!-- %CODE%DEV%PAR1 ��������� ����� �������� ���������� %NUM ����. (r) -->
		<snmp		id	= "%CODE%DEV%PAR1" 
				ptype	= "value"
				vtype   = "int"
				rexp    = ".*\s+"
				addr    = ".1.3.6.1.4.1.343.2.7.2.2.2.1.1.2.%OID" 
				fval	= "2"
				gval	= "0"
				sdep    = "%CODE%DEV001"
				update  = "5000" >
		</snmp>

		<!-- %CODE%DEV%PAR2 ��������� ����� �������� ���������� %NUM ����. (r) -->
		<calc	 	id     = "%CODE%DEV%PAR2" 
				ptype  = "value"
				vtype  = "string"
				math   = ",%CODE%DEV%PAR1">

			<rule key="0" value="���"  />
			<rule key="2" value="����" />
		</calc>
