#!/usr/bin/perl

package conf_gen_dac;

use vars qw(@ISA @EXPORT_OK);
require Exporter;

@ISA = qw(Exporter);
@EXPORT_OK = qw(get_conf);

sub read_conf {

	my $file	= shift || return 0;
	my $content_ref	= shift || return 0;

	return 0 if( !open( FILE, '<'.$file ) );
	while( <FILE> ) {
		chomp $_;
		push( @{$content_ref}, $_ );
	}
	close( FILE );

	return 1;
}

sub get_conf {

	my $file		= shift || return 0;
	my $data_ref		= shift || return 0;

 	my @content = ();
	return 0 if( !read_conf( $file, \@content ) );

	my $bFinded1 = 0; my $bFinded2 = 0; my $nCount = 0;
	my %data_1 = (); my %data_2 = ();
	my $name_component = ""; my $name_snmphost = "";
	foreach( @content ) {

		if( $_ =~ /\}/ && $bFinded1 ) { $data_ref->{$name_component} = { %data_1 };  $bFinded1 = 0; next; }
		return 0 if( $_ =~ /\{/ && $bFinded1 );

		if( $_ =~ /\{/ && !$bFinded1 ) {
			$bFinded1 = 1; $nCount = 0; %data_1 = ();
			$_ =~ s/\{\s+name\s+=\s+//; $_ =~ s/\"//g;
			$name_component = $_;
			next;
		}

		if( $bFinded1 ) {
			
		    	if( $_ =~ /\]/ && $bFinded2 ) { $data_1{$nCount} = { %data_2 }; $bFinded2 = 0; next; }
			return 0 if( $_ =~ /\[/ && $bFinded2 );
		    	
			if( $_ =~ /\[/ && !$bFinded2 ) {
				$bFinded2 = 1; $nCount++; %data_2 = ();
				$_ =~ s/\[\s+name\s+=\s+//; $_ =~ s/\"//g;
				$name_snmphost = $_;
				next;
			}
			
			if( $bFinded2 && $_ !~ /^$/ && $_ =~ /\w+/ ) {
			
				$_ =~ s/#.*//;
				my @Tmp = split( /\s+=\s+/, $_ );
				$Tmp[1] =~ s/\"//g;
				$data_2{$Tmp[0]} = $Tmp[1];
			}
		}
	}

	return 1;
}
