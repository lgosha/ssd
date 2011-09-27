#!/usr/bin/perl

# template
my $PROJECT 	= "kln4";
# template
my $TEMPLATE 	= "templates/template.XXX101.1";
# internal device code
my $CODE	= 110;

for($i=0;$i<scalar(@ARGV);$i++) {

    if($ARGV[$i] eq "--project") {
	$i++;
	$PROJECT = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--template") {
	$i++;
	$TEMPLATE = $ARGV[$i];
	next;
    }
    if($ARGV[$i] eq "--code") {
	$i++;
	$CODE = $ARGV[$i];
	next;
    }
}

# output filename
my $O_DIR 	= "tmp/$PROJECT";

open( TEMPL, '<'.$TEMPLATE ) or die "can't open template file $TEMPLATE: $!\n";
my @TEMP = <TEMPL>; close( TEMPL );

my $DEV = $TEMPLATE; $DEV =~ s/.*XXX//; $DEV =~ s/\..*//;

open( OUT, '>'.$O_DIR.'/'.$CODE.$DEV );

foreach( @TEMP ) {

    $_ =~ s/%CODE/$CODE/g;

    if( $_ =~ /%PORT_LINK/ ) {
    	my $cmd = 'perl ./sw_link_gen.pl --prj '.$PROJECT.' --code '.$CODE.' --dev '.$DEV;
	my @TMP = `$cmd`;
	foreach( @TMP ) {
		print $_;
		print OUT $_;
	}
	next;
    }

    if( $_ =~ /%ETH_LINK/ ) {
    	my $cmd = 'perl ./eth_link_gen.pl --prj '.$PROJECT.' --code '.$CODE.' --dev '.$DEV;
	my @TMP = `$cmd`;
	foreach( @TMP ) {
		print $_;
		print OUT $_;
	}
	next;
    }

    print $_;
    print OUT $_;
}
