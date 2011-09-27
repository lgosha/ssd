#!/usr/bin/perl

# project
my $PROJECT 	= "kln4";
# template
my $TEMPLATE 	= "templates/template.40HLB";
# KKS code of device
my $KKS		= "40HLB01";
# internal device code
my $CODE	= 110;
# internal code of dac component
my $SERV	= 110;

sub print_usage {

    print "update_template.pl --project project --template filename --kks code --code internal --serv serv [--help]\n";
    print "\tproject\tproject name\n";
    print "\tfilename\ttemplate filename\n";
    print "\tcode\t\tnumber of device in KKS-code\n";
    print "\tinternal\t\tinternal device code\n";
    print "\tserv\t\tinternal code of dac component\n";
    exit 0;
}

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


    open( TEMPL, '<'.'templates/'.$PROJECT.'/'.$TEMPLATE ); @TEMP = <TEMPL>; close( TEMPL );

    %comments = (); %reserve = ();
    open( OUT, '>'.'tmp/'.$PROJECT.'/'.$TEMPLATE );
    foreach( @TEMP ) {
	my $templ_type = 0;
	if( $_ =~ /%TEMPL/ ) {
	    my $tmp = $_; chomp $tmp; $tmp =~ s/%TEMPL\.//; $tmp =~ s/\s+//; $tmp =~ s/\..*//;
	    my $cmd = 'perl ./templ_gen.pl --project '.$PROJECT.' --file '.$CODE.$tmp;
	    my @TMP = `$cmd`;
	    print OUT $_ foreach( @TMP );

	    open( COMMENT, '<'.'tmp/'.$PROJECT.'/tmp.tmp.comment' ); @COMMENT = <COMMENT>; close( COMMENT );
	    foreach( @COMMENT ) {
		chomp $_;
		my @tmp = split( /\s+-\s+/, $_ );
		my $key = $tmp[0]; 
		$comments{$key} = $tmp[1];
	    }

	    open( RESERVE, '<'.'tmp/'.$PROJECT.'/tmp.tmp.reserve' ); @RESERVE = <RESERVE>; close( RESERVE );
	    foreach( @RESERVE ) {
		chomp $_;
		my @tmp = split( /\s+-\s+/, $_ );
		my $key = $tmp[0]; 
		$reserve{$key} = $tmp[1];
	    }
	}
	else {

	    $_ =~ s/%KKS/$KKS/g;
	    $_ =~ s/%SERV/$SERV/g;
	    $_ =~ s/%CODE/$CODE/g;

	    my $tmp = $_;
	    chomp $tmp; $tmp =~ s/.*id=\"//; $tmp =~ s/\".*//;

	    if( $_ =~ /%VAL/ ) {

#		$_ =~ s/%VAL/$reserve{$tmp}/;

		if( $reserve{$tmp} == -1 ) {
			$_ =~ s/%VAL//;
		}
		else {
			$_ =~ s/%VAL/ value=\"2\"/;
		}
	    }

	    if( $_ =~ /ref/ ) {
		chomp $_;
		$_ .= ' '.$comments{$tmp}."\n";
	    }

	    print OUT $_;
	}
    }
    close( OUT );
