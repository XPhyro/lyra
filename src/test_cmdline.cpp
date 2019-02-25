/*
Copyright Rene Rivera 2019
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#include <lyra/lyra.hpp>
#include "mini_test.hpp"

int main()
{
    using namespace lyra;
    bfg::mini_test::scope test;
    struct Config
    {
        std::string process_name;
        std::string file_name;
        int number = 0;
        int index = 0;
        bool flag = false;
        std::string first_pos;
        std::string second_pos;
        std::vector<std::string> unpositional;
    } config;
    auto parser
        = ExeName( config.process_name )
          | Opt( config.file_name, "filename" )
              ["-o"]["--output"]
              ( "specifies output file" )
          | Opt( config.number, "an integral value" )
              ["-n"]
          | Opt( [&]( int i ) {
                    if (i < 0 || i > 10)
                        return ParserResult::runtimeError("index must be between 0 and 10");
                    else {
                        config.index = i;
                        return ParserResult::ok( ParseResultType::Matched );
                    }
                }, "index" )
              ["-i"]
              ( "An index, which is an integer between 0 and 10, inclusive" )
          | Opt( config.flag )
              ["-f"]
              ( "A flag" )
          | Arg( config.first_pos, "first arg" )
              ( "First position" )
          | Arg( config.second_pos, "second arg" )
              ( "Second position" );
    
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-o", "filename.ext" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.process_name == "TestApp" ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-o", "filename.ext" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.file_name == "filename.ext" ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-o:filename.ext" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.file_name == "filename.ext" ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-o=filename.ext" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.file_name == "filename.ext" ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "--output", "%stdout" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.file_name == "%stdout" ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-n", "42" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.number == 42 ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-n", "forty-two" } );
        test
            (REQUIRE( !result ))
            (REQUIRE( result.errorMessage() == "Unable to convert 'forty-two' to destination type" ))
            (REQUIRE( config.number == 0 ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-i", "3" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.index == 3 ));
    }
    config = Config();
    {
        auto result = parser.parse( { "TestApp", "-i", "42" } );
        test
            (REQUIRE( !result ))
            (REQUIRE( result.errorMessage() == "index must be between 0 and 10" ));
    }
    config = Config();
    {
        auto result = parser.parse({ "TestApp", "-f" });
        test
            (REQUIRE( result ))
            (REQUIRE(config.flag));
    }
    config = Config();
    {
        auto result = parser.parse({ "TestApp" });
        test
            (REQUIRE( result ))
            (REQUIRE( result.value().type() == ParseResultType::NoMatch))
            (REQUIRE( config.flag == false ));
    }
    config = Config();
    {
        auto result = parser.parse({ "TestApp", "-f", "something" });
        test
            (REQUIRE( result ))
            (REQUIRE( config.flag ))
            (REQUIRE( config.first_pos == "something" ));
    }
    config = Config();
    {
        auto result = parser.parse({ "TestApp", "something", "-f" });
        test
            (REQUIRE( result ))
            (REQUIRE( config.flag ))
            (REQUIRE( config.first_pos == "something" ));
    }
    config = Config();
    {
        auto result = parser.parse({ "TestApp", "something" });
        test
            (REQUIRE( result ))
            (REQUIRE( config.flag == false ))
            (REQUIRE( config.first_pos == "something" ));
    }
    config = Config();
    {
        #ifdef LYRA_PLATFORM_WINDOWS
        auto result = parser.parse( { "TestApp", "/f" } );
        test
            (REQUIRE( result ))
            (REQUIRE( config.flag ));
        #endif
    }

    return test;
}