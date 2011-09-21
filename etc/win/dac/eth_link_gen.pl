#!/usr/bin/perl

# configure
my $CFG 	= 'tmp/kln4/sw';
# internal device code
my $CODE	= 110;
# internal switch code
my $DEV		= 401;
# template type
my $TYPE	= "win";

my $LINK_PAR_DIG_SHIFT = 18;
my $LINK_PAR_SYM_SHIFT = 100;

sub print_usage {

    print "conf_gen.pl --cfg filename --code internal --dev template [--help]\n";
    print "\tfilename\tports configuration filename\n";
    print "\tinternal\t\tinternal device code\n";
    print "\ttemplate\t\tdevice template code\n";
    exit 0;
}

for($i=0;$i<scalar(@ARGV);$i++) {
    if($ARGV[$i] eq "--cfg") {
	$i++;
	$CFG = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--code") {
	$i++;
	$CODE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--dev") {
	$i++;
	$DEV = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--help" || $ARGV[$i] eq "-h") {
	print_usage();
    }
}

open( CONF, '<'.$CFG ) or die "can't open config file $CONF: $!\n";
my @CFG = <CONF>; close( CONF );

my $TYPE = $CFG[0]; chomp $TYPE;
my @tmp1 = split( /\s+/, $CFG[1] );
my @tmp2 = split( /\s+/, $CFG[2] );

# template
my $TEMPLATE 	= 'templates/kln4/template.eth.link.'.$TYPE;

open( TEMPL, '<'.$TEMPLATE.'.b' ) or die "can't open template file $TEMPLATE.b: $!\n";
my @TEMP1 = <TEMPL>; close( TEMPL );
open( TEMPL, '<'.$TEMPLATE.'.r' ) or die "can't open template file $TEMPLATE.r: $!\n";
my @TEMP2 = <TEMPL>; close( TEMPL );

sub replace {

	my $line = shift;
	my $port = shift;
	my $count = shift;

	$line =~ s/%CODE/$CODE/;
	$line =~ s/%DEV/$DEV/;
	$line =~ s/%OID/$port/;
	my $par1 = sprintf("%03d", $count + $LINK_PAR_DIG_SHIFT);
	my $par2 = sprintf("%03d", $count + $LINK_PAR_DIG_SHIFT + $LINK_PAR_SYM_SHIFT);			
	$line =~ s/%PAR1/$par1/;
	$line =~ s/%PAR2/$par2/;
	$count++;
	$line =~ s/%NUM/$count/;
	
	print $line;
}

my $count = 0;
foreach my $port ( @tmp1 ) {


	if( $tmp2[$count] eq 'r' ) {
		replace( $_, $port, $count ) foreach ( @TEMP2 );
	}
	else {
		replace( $_, $port, $count ) foreach ( @TEMP1 );
	}

	$count++;	
}
