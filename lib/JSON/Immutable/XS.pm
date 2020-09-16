package JSON::Immutable::XS;
use parent 'Export::XS';
use 5.020;
use strict;
use warnings;
use XS::Framework;

our $VERSION = '0.1.0';

XS::Loader::bootstrap();

1;
