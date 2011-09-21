#!/usr/bin/perl

# template
my $TEMPLATE 	= "templates/template.40HLB";
# KKS code of device
my $KKS		= "40HLB01";
# internal device code
my $CODE	= 110;
# internal code of dac component
my $SERV	= 110;

sub print_usage {

    print "conf_gen.pl --template filename --kks code --code internal --serv serv [--help]\n";
    print "\tfilename\ttemplate filename\n";
    print "\tcode\t\tnumber of device in KKS-code\n";
    print "\tinternal\t\tinternal device code\n";
    print "\tserv\t\tinternal code of dac component\n";
    exit 0;
}

for($i=0;$i<scalar(@ARGV);$i++) {
    if($ARGV[$i] eq "--template") {
	$i++;
	$TEMPLATE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--kks") {
	$i++;
	$KKS = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--code") {
	$i++;
	$CODE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--serv") {
	$i++;
	$SERV = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--help" || $ARGV[$i] eq "-h") {
	print_usage();
    }
}

open( TEMPL, '<'.$TEMPLATE ) or die "can't open template file $TEMPLATE: $!\n";
my @TEMP = <TEMPL>; close( TEMPL );

foreach( @TEMP ) {
    $_ =~ s/%SERV/$SERV/g;
    $_ =~ s/%CODE/$CODE/g;
    $_ =~ s/%KKS/$KKS/g;
}

print @TEMP;
