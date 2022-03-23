# ArgumentParser

TODO: Learn markdown

An argument parser based on the Python ArgumentParser API. Some changes have been made to the API due to
translation constraints between C++ and Python, but the nature of the class has been maintained.

Building the options handling is simple and straightforward.
```
std::vector< std::string > inputFiles;
ArgumentParser parser;
parser.addOption( "output-file" );           // As simple as this
parser.addOption(
    "input-file", "InputFiles", true,        // Or as complex as this
    "One or more input files to be handled",
    ArgumentParser::OptionValue::required,
    ArgumentParser::OptionSelection::take_all,
    [ & ]( const std::string& value ) {
        std::string filePath;
        for ( std::stringstream stream( value );
            std::getline( stream, filePath, ',' );
            inputFiles.push_back( filePath ) );
    } );
parser.addOption(
    "--the-flag", "", false,
    "Flag that something should happen",
    ArgumentParser::OptionValue::none );
parser.parseArguments( argc, argv );
```

A few things to keep in mind:
* Option strings will be normalized to have 2 leading dashes, should they not be present.
* I'm lazy and don't feel like typing "--" for every option flag, so they're implied if not supplied.
* The option flag "--help" is reserved, with or without the leading 2 dashes, and regardless of capitalizations.
* Short option flags are not currently incorporated.
* The callback must have the following signature `void ( const std::string& )`
* The alias string must be unique and not collide with any option flag.
* The alias string is useful when getting the options via `getParsedOptions()` and I want to encode more information internally.
