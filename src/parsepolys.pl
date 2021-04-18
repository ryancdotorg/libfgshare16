#!/usr/bin/perl -w
no warnings "portable";
use strict;

# grep '      ' primitive-polynomial-table.txt | grep -v '^$' | tr -d ',*' | perl -e '$d = join("",<>);$d =~ s/\s{3,99}/ /mg;print $d' > polys

my @poly_lines = grep {/ {7}/} <>;

my $poly_data  = join('', @poly_lines);

$poly_data =~ s/[,\*]//g;
$poly_data =~ s/\s{2,99}/ /mg;

@poly_lines = split(/\n/m, $poly_data);

my %octbin = ('0' => '000', '1' => '001', '2' => '010', '3' => '011',
              '4' => '100', '5' => '101', '6' => '110', '7' => '111');

foreach my $line (@poly_lines) {
  chomp $line;
  my @polys = split(/\s+/, $line);
  my $order = shift(@polys);
  next if ($order > 63);
  @polys = map { join('', map {$octbin{$_}} split(//, $_)) } @polys;
  @polys = sort { scalar(grep {/1/} split('', $a)) <=> scalar(grep {/1/} split('', $b)) } @polys;
  @polys = map { "0b$_" } @polys;
  @polys = map { sprintf("0x%x", oct($_)) } @polys;
  print "$order\t" . join(', ', @polys) . "\n";
}


# vim: ts=2 sw=2 et ai si bg=dark
