#!/usr/bin/perl -w
#
# @file gen_headers.pl
# @brief BCON (BCON C Object Notation) Header generation
#
#   Copyright 2009-2013 MongoDB Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

use strict;

use constant MAX_ARGS => 256;

open my $bcon_enum, "> bcon_enum.h" or die "Couldn't open bcon_enum.h: $!";
open my $bcon_enum_str, "> bcon_enum_str.h" or die "Couldn't open bcon_enum_str.h: $!";
open my $bcon_sub_symbols, "> bcon_sub_symbols.h" or die "Couldn't open bcon_sub_symbols.h: $!";
open my $bcon_union, "> bcon_union.h" or die "Couldn't open bcon_union.h: $!";
open my $bcon_indirection, "> bcon_indirection.h" or die "Couldn't open bcon_indirection.h: $!";

open my $rows, "< bcon_types.txt" or die "Couldn't open bcon_types.txt for reading: $!";

while (my $row = <$rows>) {
    chomp($row);
    my ($complex_type, $name) = split /\t/, $row;

    $name = uc($name);

    print $bcon_enum join("", map { "BCONT_$_$name,\n" } ('', 'R', 'P'));
    print $bcon_enum_str join("", map { "\"BCONT_$_$name\",\n" } ('', 'R', 'P'));

    if ($complex_type) {
        print $bcon_sub_symbols join("", map { "#define BCON_$_$name(val) BCON_MAGIC, .type = BCONT_$_$name, .$_$name = (val)\n" } ('', 'R', 'P'));

        my ($type) = ($complex_type =~ /^([\w ]+)/);
        $complex_type =~ s/[^*]//g;

        my $n = length($complex_type);

        print $bcon_union "$type ", join(", ", map { ("*" x ($n++)) . "$_$name" } ('', 'R', 'P')), ";\n";

        print $bcon_indirection join("\n",
            "case BCONT_$name: *out = (void *)(&(in->$name)); break;",
            "case BCONT_R$name: *out = (void *)(in->R$name); type = BCONT_$name; break;",
            "case BCONT_P$name: *out = (void *)(*(in->P$name)); type = BCONT_$name; break;"
        ), "\n";
    } else {
        print $bcon_sub_symbols "#define BCON_$name BCON_MAGIC, .type = BCONT_$name, 0\n";
    }
}

open my $bcon_gen_macros, "> bcon_gen_macros.h" or die "Couldn't open bcon_gen_macros.h: $!";

print $bcon_gen_macros "#define BCON_MACRO_ARG(" . join(", ", map { "_$_" } (1..MAX_ARGS)) . ", ...) _" . MAX_ARGS . "\n";

print $bcon_gen_macros "#define BCON_MACRO_COUNT(...) BCON_MACRO_ARG(__VA_ARGS__, " . join(", ", reverse(0..(MAX_ARGS - 1))) . ") \n";

for (my $i = 2; $i < MAX_ARGS; $i++) {
    my $j = $i - 1;
    print $bcon_gen_macros "#define BCON_MACRO_MAP_$i(m, sep, v, ...) m(v)BCON_MACRO_CONCAT sep BCON_MACRO_MAP_$j(m, sep, __VA_ARGS__)\n";
}
