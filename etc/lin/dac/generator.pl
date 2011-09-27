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

$TMPDIR = "tmp/$project";

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

		    my $cmd = 'perl ./conf_gen.pl --template templates/'.$project.'/template.XXX'.$base.' --code '.${${$COMPONENTS{$dev}}{'code'}}[$c].' --project '.$project;
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
