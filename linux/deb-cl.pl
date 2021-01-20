#!/usr/bin/perl -w

use strict;
use warnings;

use Dpkg::Changelog::Debian;
use File::Basename;

my $c = Dpkg::Changelog::Debian->new();

$c->load("debian/changelog");
my $entry = new Dpkg::Changelog::Entry;

my $version = shift;
my $dist = shift;
my $short = shift;
my $date;
my $commit;

open GIT, "git log --date=rfc HEAD^..HEAD|";
while(<GIT>) {
	chomp;
	if(/^Date:\s+(.*)$/) {
		$date = $1;
	}
	elsif(/^commit\s(.*)$/) {
		$commit = substr($1, 0, 7);
	}
}
close GIT;

$entry->{header} = "beidconnect ($version-0" . $short . "-1) $dist; urgency=low";
$entry->{changes} = ["  * built from commit $commit"];
$entry->{trailer} = " -- BOSA servicedesk <servicedesk\@bosa.fgov.be>  $date";
$entry->normalize;
unshift @{$c->{data}}, $entry;
$c->save("debian/changelog");
print $c;
