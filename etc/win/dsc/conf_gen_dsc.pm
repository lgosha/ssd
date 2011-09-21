#!/usr/bin/perl

package conf_gen_dsc;

use vars qw(@ISA @EXPORT_OK);
require Exporter;

@ISA = qw(Exporter);
@EXPORT_OK = qw(get_conf cpy clean_dir);

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

	my $bFinded = 0;
	my %data_component = (); my $name_component = "";
	foreach( @content ) {

		if( $_ =~ /\}/ && $bFinded ) { $data_ref->{$name_component} = { %data_component }; $bFinded = 0; next; }
		return 0 if( $_ =~ /\{/ && $bFinded );

		if( $_ =~ /\{/ && !$bFinded ) {
			$bFinded = 1; %data_component = ();
			$_ =~ s/\{\s+name\s+=\s+//; $_ =~ s/\"//g;
			$name_component = $_;
			next;
		}

		if( $bFinded && $_ !~ /^$/ && $_ =~ /\w+/  ) {
			
			$_ =~ s/#.*//;
			my @Tmp = split( /\s+=\s+/, $_ );
			$Tmp[1] =~ s/\"//g;
			$data_component{$Tmp[0]} = $Tmp[1];
		}
	}

	return 1;
}

sub cpy {

	$from 		= shift || return 0;
	$to 		= shift || return 0;
	$mode 		= shift || return 0;

	open( FILE, '<'.$from ) or die "can't open file $from: $!";
	my @TMP = <FILE>; close(FILE);

	open( FILE, $mode.$to ) or die "can't open file $to: $!";
	print FILE @TMP; close(FILE);

	return 1;
}

sub clean_dir {

	$DIR = shift || return 0;
	
	opendir( DH, $DIR ); my @FILES = grep { !/^\./ } readdir( DH ); closedir( DH );
	unlink $DIR.'/'.$_ foreach( @FILES );

	return 1;	
}
