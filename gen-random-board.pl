#!/usr/bin/perl
#by Michael Peternell
#Created: 6-Dec-2017
use strict;

my $SCRIPT_NAME = $0;

sub usage {
	print "Usage: $SCRIPT_NAME width,height > outputFilePath.gol\n";
	exit(1);
}

if(@ARGV == 0) {
	usage();
}

my $arg = shift @ARGV;
if($arg eq '--help' || $arg eq '-h') {
	usage();
}

my ($w,$h) = split(",", $arg);
$w = int($w);
$h = int($h);
if($w<=0 || $h<=0) {
	usage();
}

print "$w,$h\n";
for(my $i=0; $i<$h; $i++) {
	for(my $colIdx=0; $colIdx<$w; $colIdx++) {
		my $r = rand();
		print ($r > 0.5 ? "x" : ".");
	}
	print "\n";
}
