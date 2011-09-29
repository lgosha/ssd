#!/usr/bin/perl -w

$PACKAGE = "ssd_conf";

use File::Path qw(mkpath remove_tree);
use File::Copy;

open( IN, '<'.'version' ) or die "can't open projects information file 'version'";
@PRJ = <IN>; close(IN);

print "Select project:\n";
for( my $i=0;$i<scalar(@PRJ);$i++ ) {
    my $tmp = $PRJ[$i]; chomp $tmp;  $tmp =~ s/:.*//;
    print sprintf( "%d", $i+1 ), '. ', $tmp, "\n";
}
my $tmp = <STDIN>; chomp $tmp;
if( $tmp < 1 || $tmp > scalar(@PRJ)  ) {
    print "error: wrong selection: $tmp\n";
    exit 1;
}
$tmp = $PRJ[$tmp-1]; chomp $tmp;

$PROJECT = $tmp; $PROJECT =~ s/:.*//;
$VERSION = $tmp; $VERSION =~ s/.*:\s+//;

$NAME = $PACKAGE.'.'.$PROJECT.'.'.$VERSION;


mkpath('tmp/dac/conf');
mkpath('tmp/dsc/conf');

my $DIR = "..\/dac\/conf\/$PROJECT";
opendir( DH, $DIR ) or die "can't open dir $DIR";
my @FILES = grep { !/^\./ && /\.xml/ } readdir( DH ); closedir( DH );
copy( "$DIR/$_", 'tmp/dac/conf/'.$_ ) foreach( @FILES );

open( IN, '<'.'dac_conf.nsi.template' ) or die "can't open dac_conf.nsi.template";
@T = <IN>; close(IN);
foreach( @T ) {
	$_ =~ s/%VERSION/$VERSION/;
	$_ =~ s/%PROJECT/$PROJECT/;
}
open( OUT, '>'.'dac_conf.nsi'); print OUT @T; close( OUT );

system("\"C:\\Program Files\\NSIS\\makensis.exe\" dac_conf.nsi");
unlink("dac_conf.nsi");

$DIR = "..\/dsc\/conf\/$PROJECT";
opendir( DH, $DIR ) or die "can't open dir $DIR";
@FILES = grep { !/^\./ && /\.xml/ } readdir( DH ); closedir( DH );
copy( "$DIR/$_", 'tmp/dsc/conf/'.$_ ) foreach( @FILES );

open( IN, '<'.'dsc_conf.nsi.template' ) or die "can't open dsc_conf.nsi.template";
@T = <IN>; close(IN);
foreach( @T ) {
	$_ =~ s/%VERSION/$VERSION/;
	$_ =~ s/%PROJECT/$PROJECT/;
}
open( OUT, '>'.'dsc_conf.nsi'); print OUT @T; close( OUT );

system("\"C:\\Program Files\\NSIS\\makensis.exe\" dsc_conf.nsi");
unlink("dsc_conf.nsi");

remove_tree("tmp");
