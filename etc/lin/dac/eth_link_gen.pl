#!/usr/bin/perl

# configure
my $PROJECT 	= 'kln4';
# internal device code
my $CODE	= 110;
# internal switch code
my $DEV		= 401;
# template type
my $TYPE	= "win";

my $LINK_PAR_DIG_SHIFT = 18;
my $LINK_PAR_SYM_SHIFT = 100;

for($i=0;$i<scalar(@ARGV);$i++) {
    if($ARGV[$i] eq "--prj") {
	$i++;
	$PROJECT = $ARGV[$i];
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
}

# configure
my $CFG  = 'tmp/'.$PROJECT.'/eth';

open( CONF, '<'.$CFG ) or die "can't open eth_link config file $CFG: $!\n";
my @CFG = <CONF>; close( CONF );

my $TYPE = $CFG[0]; chomp $TYPE;
my @tmp1 = split( /\s+/, $CFG[1] );
my @tmp2 = split( /\s+/, $CFG[2] );

# template
my $TEMPLATE 	= 'templates/'.$PROJECT.'/template.eth.link.'.$TYPE;

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
