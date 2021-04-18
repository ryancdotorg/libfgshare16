#!/usr/bin/perl -w
use strict;

use List::Util qw(shuffle);

my @logs;
my @exps;

my $F_SZ = $ARGV[0] || 65536;
my $LMAX = $F_SZ - 2;
my $EMAX = $F_SZ - 1;

#my $v = 0x100b;
my %poly = (
    256 =>   0x1d,
   4096 =>  0x053,
  65536 => 0x100b,
);
my $v = $poly{$F_SZ};

while ($v < $F_SZ) {
  @logs = (0)x($LMAX+1);
  @exps = (0)x($EMAX+1);
  my $x = 1;
  foreach my $i (0..$LMAX) {
    $exps[$i+$F_SZ-1] = $x;
    $exps[$i] = $x;
    $logs[$x] = $i;
    $x *= 2; # aka left shift by one
    $x ^= ($F_SZ + $v) if ($x & $F_SZ); # Unset the 8th bit and mix in 0x1d
  }
  $logs[0] = 0;

  my $okay = 1;
  foreach my $i (0..$LMAX) {
    unless ($logs[$exps[$i]] == $i) {
      #warn "bad data \$logs[\$exps[$i]] == $logs[$exps[$i]]";
      $okay = 0;
    }
  }
  foreach my $i (1..$EMAX) {
    unless ($exps[$logs[$i]] == $i) {
      #warn "bad data \$exps[\$logs[$i]] == $exps[$logs[$i]]";
      $okay = 0;
    }
  }
  last if ($okay);
  $v += 2;
}

sub randint {
  return int(rand($F_SZ));
}

sub rand16 {
  return int(rand(65536));
}

sub gf_mul {
  my $a = shift;
  while (my $b = shift) {
    return 0 if ($a == 0 || $b == 0);
    $a = $exps[$logs[$a]+$logs[$b]];
  }
  return $a;
}

sub gf_div {
  my $a = shift;
  my $b = shift;

  return 0 if ($a == 0);
  die if ($b == 0);

#  printf("mul %u %u\n", $a, $b);
  return $exps[($F_SZ-1)+$logs[$a]-$logs[$b]];
}

sub gf_add {
  my $a = shift;
  my $b = shift;

  return ($a^$b) % $F_SZ;
}

sub gf_sub {
  my $a = shift;
  my $b = shift;

  return ($a^$b) % $F_SZ;
}

sub gf_neg {
  return shift;
}

sub gf_pow2 {
  my $a = shift;
  my $b = shift;

  printf("pow2 %u %u\n", $a, $b);
  my $r = 1;
  foreach (1..$b) {
    $r = gf_mul($r, $a);
  }


  return $r;
}

# exponetiation by squaring
sub gf_pow {
  my $a = shift;
  my $b = shift;

#  printf("pow %u %u\n", $a, $b);
  my $t = 1;
  my $s = $a;
  my $m = 1;
  while (1) {
    last if ($m > $b);
    $t = gf_mul($t, $s) if ($b & $m);
    $s = gf_mul($s, $s);
    $m <<= 1;
  }

  return $t;
}

sub gf_split {
  my $secret  = shift;
  my $nshares = shift;
  my $nthresh = shift;

  my (@coef, @shares);
  my $t;
  $coef[0]   = $secret;
  $shares[0] = $secret;
  for (my $c = 1; $c < $nshares; $c++) { $coef[$c] = randint(); }

  foreach my $x (1..$nshares) {
    my $t = $coef[0];
    my $ilog = $logs[$x];
    my $xpow = 1;
    for (my $exp = 1; $exp < $nthresh; $exp++) {
      $xpow = $exps[$ilog + $logs[$xpow]] if ($xpow);
      $t = gf_add($t, gf_mul($coef[$exp], $xpow));
    }
    $shares[$x] = $t;
  }
  return @shares;
}

sub gf_join {
  my $shares = shift;

  return $shares->[0]->[1] if (scalar(@$shares) == 1);

  my $T = 0;
  for (my $formula = 0; $formula < scalar(@$shares); $formula++) {
    my $value;
    my ($Li_top, $Li_bottom) = (0, 0);
    for (my $count = 0; $count < scalar(@$shares); $count++) {
      next if ($formula == $count);
      $value       = $shares->[$formula]->[1];
      my $startpos = $shares->[$formula]->[0]; # i
      my $nextpos  = $shares->[$count]->[0];   # j
      # $log_numer = gf_mul($numer, gf_neg($nextpos))
      $Li_top    = ($Li_top + $logs[gf_neg($nextpos)]);
      #$Li_top    -= 65535 if ($Li_top > 65535);
      # $log_denom = gf_mul($denom, gf_sub($startpos, $nextpos))
      $Li_bottom = ($Li_bottom + $logs[gf_sub($startpos, $nextpos)]);
      #$Li_bottom -= 65535 if ($Li_bottom > 65535);
    }
    $Li_top    %= ($F_SZ-1);
    $Li_bottom %= ($F_SZ-1);
    $Li_top += ($F_SZ-1) if ($Li_bottom > $Li_top);
    #my $Li_div = $Li_top - $Li_bottom; # $exps[$numer / $denom]
    
    $T ^= ($exps[$logs[$value] + ($Li_top - $Li_bottom)]) if ($value);
  }
  return $T;
}

foreach my $i (0..255) {
  printf("%02x * 2 = %02x\n", $i, gf_mul($i, 2));
  printf("%02x / 2 = %02x\n", $i, gf_div($i, 2));
}
printf("%u\n", gf_mul(0, 2));
printf("%u\n", gf_mul(1, 2));
printf("%u\n", gf_mul(2, 2));
printf("%u\n", gf_mul(3, 2));

printf("%u\n", gf_mul(5, 5));
printf("%u\n", gf_div(gf_mul(17, 5), 5));
printf("%u\n", gf_pow(5, 5));
printf("%u\n", gf_pow(3, 2));
printf("%u\n", gf_pow(2, 5));
printf("%u\n", gf_pow(2, 15));
printf("%u\n", gf_pow(2, 16));
printf("%u\n", gf_pow(2, 17));
if ($F_SZ >= 4096) {
  printf("%u\n", gf_pow(2, 4094));
  printf("%u\n", gf_pow(2, 4095));
  printf("%u\n", gf_pow(7, 1337));
}
if ($F_SZ >= 65536) {
  printf("%u\n", gf_pow(2, 65534));
  printf("%u\n", gf_pow(2, 65535));
  printf("%u\n", gf_pow(7, 31337));
}
exit 0;

foreach my $n (0..($F_SZ-1)) {
  my $okay = 1;
  foreach my $s (6, 10, 321, 1337) {
    foreach my $t (1, 2, 3, 6, 10) {
      next if ($t > $s);
      my $p = 0;
      my $shares = [map { [$p++, $_] } gf_split($n, $s, $t)];
      shift @$shares;
      #print join(', ', map { $_->[1] } @$shares) . "\n";
      foreach (0..9) {
        $okay = 0 if (gf_join([(shuffle(@$shares))[0..($t-1)]]) != $n);
      }
    }
  }
  printf("%5d: %s\n", $n, $okay ? 'PASS' : 'FAIL');
}
# vim: ts=2 sw=2 et ai si bg=dark
