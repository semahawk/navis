#
#
# build_error_pages.pl
#
# Created at:  Thu 02 May 2013 14:06:51 CEST 14:06:51
#
# Author:  Szymon Urbaś <szymon.urbas@aol.com>
#
# License: the MIT license
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#

use strict;
use warnings;

if (scalar @ARGV < 1){
  print STDERR "build_error_pages.pl: at least one argument expected\n";
  exit 1;
}

my $version = $ARGV[0];
my %errors =
(
  "404", "Not Found",
  "500", "Internal Server Error",
  "503", "Bad Gateway"
);

# let's make sure there is the "conf" directory
if (! -d "conf"){
  system("mkdir conf");
}

foreach my $error (sort keys %errors){
  my $code = $error;
  my $desc = $errors{$error};
  my $content = <<EOF;
<!doctype html>
<html>
  <head>
    <title>$code $desc</title>
  </head>
  <body align="center">
    <pre style="font-size: 120px;">):</pre>
    <div  style="font-size: 2em;">
      "$desc"
    </div>
    <div style="margin-top: 10px; font-size: 14px; font-family: serif; font-style: italic;">
      says Navis $version
    </div>
  </body>
</html>
EOF

  system("touch conf/".$code.".html");
  system("echo '".$content."' > conf/".$code.".html");
}

