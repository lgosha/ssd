#!/usr/bin/perl

use conf_gen_dsc qw(get_conf cpy clean_dir);

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

# devices
#%DEV = (
#	'40HLB01'	=> [ '40HLB',		'110', '110'  ],
#	'40HLB02',	=> [ '40HLB',		'120', '120'  ],
#	'40HLB03_01',	=> [ '40HLB.1', 	'131', '131'  ],
#	'40HLB04_01',	=> [ '40HLB.1', 	'141', '141'  ],
#	'40HLB05_01',	=> [ '40HLB.1', 	'151', '151'  ],
#	'40HLB06_01',	=> [ '40HLB.1', 	'161', '161'  ],
#	'40HLB03_02',	=> [ '40HLB.2', 	'132', '132'  ],
#	'40HLB04_02',	=> [ '40HLB.2', 	'142', '142'  ],
#	'40HLB05_02',	=> [ '40HLB.2', 	'152', '152'  ],
#	'40HLB06_02',	=> [ '40HLB.2', 	'162', '162'  ],
#	'40HLC01',	=> [ '40HLC',		'201', '201'  ],
#	'40HLC02',	=> [ '40HLC',		'202', '202'  ],
#	'40HLD01_01',	=> [ '40HLD.1',		'301', '301'  ],
#	'40HLE01',	=> [ '40HLE.011',	'401', '201'  ],
#	'40HLE02',	=> [ '40HLE.011',	'402', '202'  ],
#	'40HLE03',	=> [ '40HLE.012-03',	'403', '131'  ],
#	'40HLE04',	=> [ '40HLE.012-03',	'404', '141'  ],
#	'40HLE15',	=> [ '40HLE.012-03',	'415', '120'  ],
#	'40HLE16',	=> [ '40HLE.012-03',	'416', '120'  ],
#	'40HLE05',	=> [ '40HLE.012',	'405', '131'  ],
#	'40HLE06',	=> [ '40HLE.012',	'406', '141'  ],
#	'40HLE08',	=> [ '40HLE.012',	'408', '142'  ],
#	'40HLE10',	=> [ '40HLE.012',	'410', '161'  ],
#	'40HLE11',	=> [ '40HLE.012',	'411', '152'  ],
#	'40HLE07',	=> [ '40HLE.012-01',	'407', '132'  ],
#	'40HLE09',	=> [ '40HLE.012-01',	'409', '151'  ],
#	'40HLE17',	=> [ '40HLE.012-01',	'417', '162'  ],
#	'40HLE12',	=> [ '40HLE.013',	'412', '152'  ],
#	'40HLE13',	=> [ '40HLE.013',	'413', '162'  ],
#	'40HLE14'	=> [ '40HLE.012-02',	'414', '110'  ],
#	);

	# 110	- 110, 414
	# 120	- 120, 415, 416
	# 131	- 131, 403, 405
	# 132	- 132, 407
	# 141	- 141, 404, 406
	# 142	- 142, 408
	# 151	- 151, 409
	# 152	- 152, 411, 412
	# 161	- 161, 410
	# 162	- 162, 413, 417
	# 201	- 201, 401
	# 202	- 202, 402
	# 301	- 301

print 'clean temporary dir', "\n";
clean_dir( 'tmp/'.$project );
print 'generate base', "\n";
foreach my $component ( sort keys %conf ) {

    my $template = ""; my $prefix = ""; my $dac_prefix = "";
    foreach( %{$conf{$component}} ) {
	
	$template 	= ${$conf{$component}}{$_}	if( $_ =~ /template/ );
	$prefix 	= ${$conf{$component}}{$_}	if( $_ =~ /prefix/ );
	$dac_prefix	= ${$conf{$component}}{$_} 	if( $_ =~ /dac_prefix/ );
		
    }

    print $component, "\n";
    system( 'perl ./update_template.pl --project '.$project.'  --template '.'template.'.$template.' --kks '.$component.' --code '.$prefix.' --serv '.$dac_prefix );
    system( 'perl ./conf_gen.pl --template '.'tmp/'.$project.'/template.'.$template.' --kks '.$component.' --code '.$prefix.' --serv '.$dac_prefix.' > tmp/'.$project.'/'.$component );
    cpy( 'tmp/'.$project.'/'.$component, 'tmp/'.$project.'/dsc', '>>' );
}

print 'generate configure', "\n";
opendir( DH, "templates/$project" ); my @FILES = grep { !/^\./ && /\.xml/ } readdir( DH ); closedir( DH );
open( BASE, '<'.'tmp/'.$project.'/dsc' ); my @BASE = <BASE>; close( BASE );

foreach my $f ( @FILES ) {
    open( IN, '<'.'templates/'.$project.'/'.$f ); my @TEMP = <IN>; close( IN );
    my $file = $f; $file =~ s/^template\.//;
    open( OUT, '>'.'conf/'.$project.'/'.$file );
    foreach( @TEMP ) {
	if( $_ =~ /%BASE/ ) {
	    print OUT $_ foreach( @BASE );
	}
	else { 
	    print OUT $_;
	}
    }
    close( OUT );
}

print 'mix blink signals', "\n";
%mix_blink = (
	    '131'	=> '132',
	    '141'	=> '142',
	    '151'	=> '152',
	    '161'	=> '162',
);

opendir( DH, "conf/$project" ); my @FILES = grep { !/^\./ && /\.xml/ } readdir( DH ); closedir( DH );

foreach my $f ( @FILES ) {

    open( IXML, '<'.'conf/'.$project.'/'.$f ); my @DATA = <IXML>; close( IXML );
    open( OXML, '>'.'conf/'.$project.'/'.$f ); 
    my $inside = 0; my $collect = 0; my $insert = 0; my $finded1 = 0; my $finded2 = 0;
    %mix_data = ();
    foreach my $line ( @DATA ) {

        if( $line =~ /BLINK/ ) {
		$inside = 1;
        }
    
	if( $inside && $line =~ /\<\/indicator/ ) {
	    my $continue = 0;
	    $continue = 1 if( $collect && !$insert );
	    $inside = 0; $collect = 0; $insert = 0;
	    next if( $continue );
	}	
    
	if( $inside ) {
    
	    if( !$collect ) {
		    foreach( keys %mix_blink ) {
			if( $line =~ /\($_\)/  ) { $collect = 1; $finded1 = $_;  break; }
		    }
	    }
	
	    if( $collect ) {
	
		if( $line =~ /ref/ || $line =~ /\<\!\-\-/ && $line !~ /BLINK/ ) { push( @{$mix_data{$finded1}}, $line );  }
		next;
	    }
	    else {
	
		if( !$insert ) {
			foreach( values %mix_blink ) {
			        if( $line =~ /\($_\)/  ) { $insert = 2; $finded2 = $_;  break; }
			}
		}
		
		if( $insert == 2 ) {
		
		    if( $line =~ /BLINK/ ) {
			my @tmp = split( /\s+/, $line );
			my $host = $tmp[3]; $host =~ s/_.*//;
			print OXML "\t<!-- BLINK $host ($finded1) ($finded2) -->\n";
			next;
		    }

		    if( $line =~ /\<indicator/ ) {
			$line =~ s/id=\"$finded2/id=\"$finded1/;
		    }
		    
		    if( $line =~ /\<\!\-\-/ ) {
			foreach( @{$mix_data{$finded1}} ) { 
			    if( $_ =~ /БС/ ) { print OXML "\t\t    <!-- БС ($finded1) -->\n"; }
			    else { print OXML $_; }
			}
			$insert = 1;
		    }
		}
		
		if( $insert == 1 ) {
			if( $line =~ /БС/ ) { print OXML "\t\t    <!-- БС ($finded2) -->\n"; next; }
		}
	    }
	}

	print OXML $line;
    }
    
    close( OXML );
}

print "done", "\n";
