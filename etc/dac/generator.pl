#!/usr/bin/perl

use conf_gen_dac qw(get_conf);

sub print_usage {

    print "genarator.pl --project <project_name>", "\n";
}

my $project = "kln4";

for($i=0;$i<scalar(@ARGV);$i++) {

    SWITCH : {

        if($ARGV[$i] eq "--project" || $ARGV[$i] eq "-p") {
	
	    $i++; $project = $ARGV[$i];
	    next;
	}
	
        if($ARGV[$i] eq "--help" || $ARGV[$i] eq "-h") {
	
		print_usage();
		last SWITCH;
	}

	print_usage();
    }
}

my $file = "etc/".$project.".conf";

my %conf = ();
if( !get_conf( $file, \%conf ) ) {
    print "can't open configuration \'$file\' for project \'$project\'", "\n";
}

mkdir 'conf';
mkdir 'tmp';
mkdir 'tmp/'.$project;
mkdir 'conf/'.$project;

my %COMPONENTS = (); my %SNMPHOST = (); 
my @host = (); my @base = (); my @code = ();
foreach my $component ( keys %conf ) {

	my @host = (); my @base = (); my @code = (); my %oids = (); my %rsrv = (); my %type = (); my %port = ();
	my %SNMPHOST = (); 
	
#	print $component, "\n";

	foreach my $snmphost ( sort keys %{$conf{$component}} ) {
	
#		print "\t", $snmphost, "\n";
			
		my $code = ''; my $base = ''; my $oids = ''; my $rsrv = ''; my $type = ''; my $port = '';
		foreach( keys %{${$conf{$component}}{$snmphost}}  ) {
			
			if( $_ =~ /template/ ) { $base = ${${$conf{$component}}{$snmphost}}{$_}; push( @base, $base ); }
			if( $_ =~ /prefix/ ) { $code = ${${$conf{$component}}{$snmphost}}{$_}; push( @code, $code ); }

			push( @host, ${${$conf{$component}}{$snmphost}}{$_} ) if( $_ =~ /host/ );

			if( $_ =~ /oids/ ) {
				$oids = ${${$conf{$component}}{$snmphost}}{$_};
			}
			if( $_ =~ /reserve/ ) {
				$rsrv = ${${$conf{$component}}{$snmphost}}{$_};				
			}
			if( $_ =~ /type/ ) {
				$type = ${${$conf{$component}}{$snmphost}}{$_};				
			}
			if( $_ =~ /ports/ ) {
				$port = ${${$conf{$component}}{$snmphost}}{$_};				
			}
		
#			print "\t\t", $_, "\t-\t", ${${$conf{$component}}{$snmphost}}{$_}, "\n";
			
		}

		my $tmp = $base; $tmp =~ s/\..*//;

		$oids{$code.$tmp} = $oids;				
		$rsrv{$code.$tmp} = $rsrv;				
		$type{$code.$tmp} = $type;
		$port{$code.$tmp} = $port;				
	}
	
	$SNMPHOST{'host'} = [ @host ];
	$SNMPHOST{'base'} = [ @base ];
	$SNMPHOST{'code'} = [ @code ];
	$SNMPHOST{'oids'} = { %oids };
	$SNMPHOST{'rsrv'} = { %rsrv };
	$SNMPHOST{'type'} = { %type };
	$SNMPHOST{'port'} = { %port };
	
	$COMPONENTS{$component} = { %SNMPHOST };
}

#%DEV = (
#
#	"40HLB01"	=>	{
#				    "host"	=> [ "10.27.14.78", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "411", "412", "413", "414" ],
#				    "code"	=> [ "110", "110", "110", "414", "414", "414", "414", "414" ],
#				},
#	"40HLB02"	=>	{
#				    "host"	=> [ "10.27.14.79", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "415", "416", "301.1", "415", "416" ],
#				    "code"	=> [ "120", "120", "120", "415", "415", "415", "416", "416", "416" ],
#				},
#	"40HLB03_01"	=>	{
#				    "host"	=> [ "10.27.14.80", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "415", "416", "301.1", "403", "404", "405", "406", "407" ],
#				    "code"	=> [ "131", "131", "131", "403", "403", "403", "405", "405", "405", "405", "405", "405" ],
#				},
#	"40HLB03_02"	=>	{
#				    "host"	=> [ "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "301.1", "408", "409", "410" ],
#				    "code"	=> [ "132", "407", "407", "407", "407" ],
#				},
#	"40HLB04_01"	=>	{
#				    "host"	=> [ "10.27.14.82", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "415", "416", "301.1", "403", "404", "405", "406", "407" ],
#				    "code"	=> [ "141", "141", "141", "404", "404", "404", "406", "406", "406", "406", "406", "406" ],
#				},
#	"40HLB04_02"	=>	{
#				    "host"	=> [ "10.27.14.83", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "301.1", "403", "404", "405", "406", "407" ],
#				    "code"	=> [ "142", "408", "408", "408", "408", "408", "408" ],
#				},
#	"40HLB05_01"	=>	{
#				    "host"	=> [ "10.27.14.84", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "408", "409", "410" ],
#				    "code"	=> [ "151", "151", "151", "409", "409", "409", "409" ],
#				},
#	"40HLB05_02"	=>	{
#				    "host"	=> [ "10.27.14.85", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "301.1", "403", "404", "405", "406", "407", "301.2", "417" ],
#				    "code"	=> [ "152", "411", "411", "411", "411", "411", "411", "412", "412" ],
#				},
#	"40HLB06_01"	=>	{
#				    "host"	=> [ "10.27.14.86", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1", "301.1", "403", "404", "405", "406", "407" ],
#				    "code"	=> [ "161", "161", "161", "410", "410", "410", "410", "410", "410" ],
#				},
#	"40HLB06_02"	=>	{
#				    "host"	=> [ "10.27.14.87", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "301.1", "408", "409", "410", "301.2", "417" ],
#				    "code"	=> [ "162", "417", "417", "417", "417", "413", "413" ],
#				},
#	"40HLC01"	=>	{
#				    "host"	=> [ "10.27.14.71", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.1", "201", "301.1", "201", "301.1", "401", "402" ],
#				    "code"	=> [ "201", "201", "201", "401", "401", "401", "401" ],
#				},
#	"40HLC02"	=>	{
#				    "host"	=> [ "10.27.14.72", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.1", "201", "301.1", "201", "301.1", "401", "402" ],
#				    "code"	=> [ "202", "202", "202", "402", "402", "402", "402" ],
#				},
#	"40HLD01_01"	=>	{
#				    "host"	=> [ "10.27.14.88", "10.27.14.81", "10.27.14.81" ],
#				    "base"	=> [ "101.2", "201", "301.1" ],
#				    "code"	=> [ "301", "301", "301" ],
#				},
#	"40HLD01_02"	=>	{
#				    "host"	=> [ "10.27.14.89" ],
#				    "base"	=> [ "101.2" ],
#				    "code"	=> [ "302" ],
#
#				}
#);

my $TMPDIR = 'tmp/kln4';

sub prepare_sw_cfg {

	my $hash = shift;
	my $key = shift;

	my $oids = ${$hash->{'oids'}}{$key};
	my $port = ${$hash->{'port'}}{$key};
	my $rsrv = ${$hash->{'rsrv'}}{$key};

    	die "can't generate configure for switch $key, bad config" if( $oids eq '' && $port eq '' );
	my @tmp1 = ();
	if( $oids eq '' ) {
		push( @tmp1, $_ ) for (1..$port);
	}
	else {
		@tmp1 = split(/\s*,\s*/, $oids);	
	}
	my @tmp2 = split(/\s*,\s*/, $rsrv);
	my $count = 0;
	my $p = ''; my $s = '';
	foreach ( @tmp1 ) {
		$count++;
		$p .= $_; $p .= ' ' if( $count != scalar(@tmp1) );
		$s .= scalar(grep( /^$count$/, @tmp2 )) ? 'r' : 'b'; $s .= ' ' if( $count != scalar(@tmp1) );
	}

	open( SW, '>'.$TMPDIR.'/sw' );
	print SW $p, "\n";
	print SW $s, "\n";	
	close( SW );
}

sub prepare_eth_cfg {

	my $type = shift;
	my $hash = shift;
	my $key = shift;

	my $oids = ${$hash->{'oids'}}{$key};
	my $port = ${$hash->{'port'}}{$key};
	my $rsrv = ${$hash->{'rsrv'}}{$key};

    	die "can't generate configure for switch $key, bad config" if( $oids eq '' && $port eq '' );
	my @tmp1 = ();
	if( $oids eq '' ) {
		push( @tmp1, $_ ) for (1..$port);
	}
	else {
		@tmp1 = split(/\s*,\s*/, $oids);	
	}
	my @tmp2 = split(/\s*,\s*/, $rsrv);
	my $count = 0;
	my $p = ''; my $s = '';
	foreach ( @tmp1 ) {
		$count++;
		$p .= $_; $p .= ' ' if( $count != scalar(@tmp1) );
		$s .= scalar(grep( /^$count$/, @tmp2 )) ? 'r' : 'b'; $s .= ' ' if( $count != scalar(@tmp1) );
	}

	open( ETH, '>'.$TMPDIR.'/eth' );
	print ETH $type, "\n";
	print ETH $p, "\n";
	print ETH $s, "\n";	
	close( ETH );
}

foreach my $dev ( sort keys %COMPONENTS ) {

	print $dev, "\n";

	# open common template
	open( TEMPL_C, '<'.'templates/'.$project.'/template.common' ) or die "can't open common template file templates/".$project."/template.common: $!\n";
	my $count = 0;
        my %TEMP_C = ();
        foreach( <TEMPL_C> ) {
    	    $TEMP_C{sprintf("%4d", $count)} = $_;
	    $count++;
        }
	close( TEMPL_C );

	foreach my $number ( sort keys %TEMP_C ) {
	    if( $TEMP_C{$number} =~ /%CODE/ ) {
		$TEMP_C{$number} =~ s/%CODE/${${$COMPONENTS{$dev}}{'code'}}[0]/;
	    }
	    if( $TEMP_C{$number} =~ /%HOST/ ) {
		my $count = -1;
		foreach my $host ( @{${$COMPONENTS{$dev}}{'host'}} ) { 
		    $count++; my $code = ${${$COMPONENTS{$dev}}{'base'}}[$count]; $code =~ s/\..*//;
	    	    $TEMP_C{$count ? $number.sprintf("%4d", $count) : $number} = "\t\t".'<host id="'.${${$COMPONENTS{$dev}}{'code'}}[$count].$code.'" addr="'.$host.'" />'."\n";
		}
	    }
	    if( $TEMP_C{$number} =~ /%BASE/ ) {
		my $count = -1; my $c = -1;
		foreach my $base ( @{${$COMPONENTS{$dev}}{'base'}} ) { 
		    $c++; my $code = $base; $code =~ s/\..*//;

		    my $key = ${${$COMPONENTS{$dev}}{'code'}}[$c].$code;

		    if( ${${$COMPONENTS{$dev}}{'type'}}{$key} eq 'sw' )  {
			prepare_sw_cfg( \%{$COMPONENTS{$dev}}, $key );
		    }		

		    if( ${${$COMPONENTS{$dev}}{'type'}}{$key} =~ /sb/ )  {

			my $tmp = ${${$COMPONENTS{$dev}}{'type'}}{$key}; $tmp =~ s/sb_//;
			prepare_eth_cfg( $tmp, \%{$COMPONENTS{$dev}}, $key );
		    }		

		    my $cmd = 'perl ./conf_gen.pl --template templates/'.$project.'/template.XXX'.$base.' --code '.${${$COMPONENTS{$dev}}{'code'}}[$c];
		    my @TEMP = `$cmd`;
		    $count++; $TEMP_C{$count ? $number.sprintf("%4d", $count) : $number} = "\n";
		    foreach( @TEMP  ) {
			$count++; $TEMP_C{$number.sprintf("%4d", $count)} = $_;
		    }
	   
		}
		$count++; $TEMP_C{$number.sprintf("%4d", $count)} = "\n";
	    }
	}

	open( OUT, '>'.'conf/'.$project."/".$dev.'.xml' );
	foreach( sort keys %TEMP_C ) {
	    print OUT $TEMP_C{$_};
	}
	close( OUT );
}
