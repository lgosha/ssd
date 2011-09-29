#!/usr/bin/perl

use conf_gen_dsc qw(cpy);

$project = 'kln4';

for($i=0;$i<scalar(@ARGV);$i++) {

    if($ARGV[$i] eq "--project") {
	$i++;
	$project = $ARGV[$i];
	next;
    }

    if($ARGV[$i] eq "--file") {
	$i++;
	$FILE = $ARGV[$i];
	next;
    }
}

$SRC_PATH_TEMPL = '../dac/templates/'.$project;
$SRC_PATH = '../dac/tmp/'.$project;
$DST_PATH = 'tmp/'.$project;

cpy( $SRC_PATH_TEMPL.'/template.header', $SRC_PATH.'/template.header', '>' );
cpy( $SRC_PATH_TEMPL.'/template.footer', $SRC_PATH.'/template.footer', '>' );

cpy( $SRC_PATH.'/template.header', $DST_PATH.'/tmp.tmp', '>' );
cpy( $SRC_PATH.'/'.$FILE, $DST_PATH.'/tmp.tmp', '>>' );
cpy( $SRC_PATH.'/template.footer', $DST_PATH.'/tmp.tmp', '>>' );

system( 'templ_gen/templ_gen.bin --file '.$DST_PATH.'/tmp.tmp' );
