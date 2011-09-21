#!/usr/bin/perl

# template
my $TEMPLATE 	= "templates/template.XXX101.1";
# output filename
my $O_DIR 	= "tmp/kln4";
# internal device code
my $CODE	= 110;

sub print_usage {

    print "conf_gen.pl --template filename --code internal --out dir [--help]\n";
    print "\tfilename\ttemplate filename\n";
    print "\tinternal\t\tinternal device code\n";
    exit 0;
}

for($i=0;$i<scalar(@ARGV);$i++) {
    if($ARGV[$i] eq "--template") {
	$i++;
	$TEMPLATE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--out") {
	$i++;
	$O_DIR = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--code") {
	$i++;
	$CODE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--help" || $ARGV[$i] eq "-h") {
	print_usage();
    }
}

open( TEMPL, '<'.$TEMPLATE ) or die "can't open template file $TEMPLATE: $!\n";
my @TEMP = <TEMPL>; close( TEMPL );

my $DEV = $TEMPLATE; $DEV =~ s/.*XXX//; $DEV =~ s/\..*//;

open( OUT, '>'.$O_DIR.'/'.$CODE.$DEV );

foreach( @TEMP ) {

    $_ =~ s/%CODE/$CODE/g;

    if( $_ =~ /%LINK_PORTS/ ) {
    	my $cmd = 'perl ./ports_gen.pl --cfg tmp/kln4/sw --code '.$CODE.' --dev '.$DEV;
	my @TMP = `$cmd`;
	foreach( @TMP ) {
		print $_;
		print OUT $_;
	}
    }
    else {
	print $_;
	print OUT $_;
    }
}
