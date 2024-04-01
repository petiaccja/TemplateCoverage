//------------------------------------------------------------------------------
// The MIT License (MIT)
// Copyright (c) 2024 Péter Kardos
//------------------------------------------------------------------------------

#define CATCH_CONFIG_RUNNER
#include <iostream>

#include <catch2/catch_session.hpp>


int main(int argc, char* argv[]) {
    int result = Catch::Session().run(argc, argv);

    return result;
}