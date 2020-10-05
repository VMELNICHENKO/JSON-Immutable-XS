use 5.022;
use Config;
use XS::Install;

write_makefile(
    NAME     => 'JSON::Immutable::XS',
    AUTHOR   => 'Vladimir Melnichenko',
    BIN_DEPS => ['Export::XS', 'XS::Framework', 'XS::libpanda'],
    BIN_SHARE => { INCLUDE => { 'src' => '/', 'external' => '/' } },
    CPLUS    => 17,
    SRC      => ['src', 'external'],
    INC      => '-Isrc -Iexternal',
    CCFLAGS  => '-Wall -Wextra',
    OPTIMIZE => '-O2',
    META_MERGE => {
        "meta-spec" => { version => 2 },
        resources => {
            repository => {
                type => 'git',
                url => 'git://github.com/VMELNICHENKO/JSON-Immutable-XS.git',
                web => 'https://github.com/VMELNICHENKO/JSON-Immutable-XS',
            },
        },
    },
);
