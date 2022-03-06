/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

// Standard includes
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Conditional includes
#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

/*
 * Notes:
 *   - Required at a minimum C++14
 */

/**
 * This class is responsible for holding the values
 * associated with a parsed option flag.
 */
class OptionArgument
{
private:

	friend class ArgumentParser;

	std::string mOptionString;
	std::string mValueName;
	std::vector< std::string > mOptionValues;

	// move assign
	void _moveAssign(
		OptionArgument&& other )
	{
		mOptionString = std::move( other.mOptionString );
		mValueName = std::move( other.mValueName );
		mOptionValues = std::move( other.mOptionValues );
	}

	// copy assign
	void _copyAssign(
		const OptionArgument& other )
	{
		mOptionString = other.mOptionString;
		mValueName = other.mValueName;
		mOptionValues = other.mOptionValues;
	}

	// assignment constructor; this'll be called by ArgumentParser
	OptionArgument(
		const std::string& optionString,
		const std::string& valueName = std::string(),
		const std::vector< std::string >& optionValues = std::vector< std::string >() )
	{
		mOptionString = optionString;
		mValueName = valueName;
		mOptionValues = optionValues;
	}

public:

	/**
	 * Default constructor to an empty OptionArgument instance.
	 */
	OptionArgument()
	{
	}

	/**
	 * Move constructor.
	 * @param other R-Value to OptionArgument to move to this instance.
	 */
	OptionArgument(
		OptionArgument&& other )
	{
		_moveAssign( std::move( other ) );
	}

	/**
	 * Copy constructor.
	 * @param other Const reference to the OptionArgument to copy to this instance.
	 */
	OptionArgument(
		const OptionArgument& other )
	{
		_copyAssign( other );
	}

	/**
	 * Move assignment operator.
	 * @param other R-Value to OptionArgument to move to this instance.
	 * @return Reference to this OptionArgument is returned.
	 */
	OptionArgument& operator=(
		OptionArgument&& other )
	{
		if ( this != &other )
		{
			_moveAssign( std::move( other ) );
		}

		return *this;
	}

	/**
	 * Copy assignment operator.
	 * @param other Const reference to the OptionArgument to copy to this instance.
	 * @return Reference to this OptionArgument is returned.
	 */
	OptionArgument& operator=(
		const OptionArgument& other )
	{
		if ( this != &other )
		{
			_copyAssign( other );
		}

		return *this;
	}

	/**
	 * The option flag that this option argument came from.
	 * @return The string of the option flag.
	 */
	const std::string& optionString() const
	{
		return mOptionString;
	}

	/**
	 * The number of values present for this option.
	 * If the option has been set to take only the first or the
	 * last, then this method is expected to only return at most 1.
	 * @return The number of values present for this option is returned.
	 */
	size_t size() const
	{
		return mOptionValues.size();
	}

	/**
	 * Get the value at {@param index}.
	 * If this option has no arguments or if the index is greater than
	 * the number of values, then this method throws std::out_of_range.
	 * @param index Index of value to retrieve. [default: 0]
	 * @return Const reference to the std::string containing the value.
	 * @throw std::out_of_range is thrown if no value exists at the given index.
	 */
	const std::string& value(
		size_t index = 0 ) const
	{
		return mOptionValues.at( index );
	}

	/**
	 * The name that this option is associated with.
	 * @return Const reference to the name that this option is associated with.
	 */
	const std::string& valueName() const
	{
		return mValueName;
	}
};

/**
 * This class is responsible for building a command line argument parser
 * and being called upon to parse and handle command line arguments.
 */
class ArgumentParser
{
public:

	/**
	 * This enumeration flags whether or not a value
	 * is required, optional, or not expected for a given option flag.
	 */
	enum class OptionValue : int
	{
		none,      ///< No value is expected for the option flag.
		optional,  ///< Any value present is optional for the option flag.
		required   ///< A value is required for the option flag.
	};

	/**
	 * This enumeration flags which value to take
	 * should an option flag be present more than once.
	 */
	enum class OptionSelection : int
	{
		take_first,  ///< Take only the first value for the option flag.
		take_last,   ///< Take only the last value for the option flag.
		take_all     ///< Take all values for the option flag.
	};

private:

	class _OptionHandler
	{
	public:

		std::string defaultStringValue;
		std::string valueName;
		std::function< void( const std::string& ) > callback;
		ArgumentParser::OptionValue valueRequired;
		ArgumentParser::OptionSelection selection;
		std::string helpString;
		bool requiredOption;

	private:

		// move assignment
		void _moveAssign(
			_OptionHandler&& other )
		{
			this->defaultStringValue = std::move( other.defaultStringValue );
			this->valueName = std::move( other.valueName );
			this->callback = std::exchange( other.callback, nullptr );
			this->valueRequired = std::exchange( other.valueRequired, ArgumentParser::OptionValue::optional );
			this->selection = std::exchange( other.selection, ArgumentParser::OptionSelection::take_last );
			this->helpString = std::move( other.helpString );
			this->requiredOption = std::exchange( other.requiredOption, false );
		}

		// copy assignment
		void _copyAssign(
			const _OptionHandler& other )
		{
			this->defaultStringValue = other.defaultStringValue;
			this->valueName = other.valueName;
			this->callback = other.callback;
			this->valueRequired = other.valueRequired;
			this->selection = other.selection;
			this->helpString = other.helpString;
			this->requiredOption = other.requiredOption;
		}

	public:

		// default constructor
		_OptionHandler()
		{
			this->defaultStringValue = std::string( "" );
			this->valueName = std::string( "" );
			this->callback = nullptr;
			this->valueRequired = ArgumentParser::OptionValue::optional;
			this->selection = ArgumentParser::OptionSelection::take_last;
			this->helpString = std::string( "" );
			this->requiredOption = false;
		}

		// move constructor
		_OptionHandler(
			_OptionHandler&& other )
		{
			_moveAssign( std::move( other ) );
		}

		// copy constructor
		_OptionHandler(
			const _OptionHandler& other )
		{
			_copyAssign( other );
		}

		// move assignment operator
		_OptionHandler& operator=(
			_OptionHandler&& other )
		{
			if ( this != &other )
			{
				_moveAssign( std::move( other ) );
			}

			return *this;
		}

		// copy assignment operator
		_OptionHandler& operator=(
			const _OptionHandler& other )
		{
			if ( this != &other )
			{
				_copyAssign( other );
			}

			return *this;
		}
	};

	// Set - Application description
	std::string mApplicationDescription;

	// Set - Options to be handled
	std::map< std::string, _OptionHandler > mOptionsHandlerMap;
	std::set< std::string > mOptionsValueNames;

	// Parsed - Options parsed
	std::map< std::string, bool > mRequiredOptions;
	std::map< std::string, OptionArgument > mParsedOptions;
	std::vector< std::string > mNonOptionArguments;

	// Move assignment
	void _moveAssign(
		ArgumentParser&& other )
	{
		mApplicationDescription = std::move( other.mApplicationDescription );
		mOptionsHandlerMap = std::move( other.mOptionsHandlerMap );
		mOptionsValueNames = std::move( other.mOptionsValueNames );
		mRequiredOptions = std::move( other.mRequiredOptions );
		mParsedOptions = std::move( other.mParsedOptions );
		mNonOptionArguments = std::move( other.mNonOptionArguments );
	}

	// Copy assignment
	void _copyAssign(
		const ArgumentParser& other )
	{
		mApplicationDescription = other.mApplicationDescription;
		mOptionsHandlerMap = other.mOptionsHandlerMap;
		mOptionsValueNames = other.mOptionsValueNames;
		mRequiredOptions = other.mRequiredOptions;
		mParsedOptions = other.mParsedOptions;
		mNonOptionArguments = other.mNonOptionArguments;
	}

	// Print the help message
	void _printHelp(
		const char* application,
		const std::vector< std::string >& missingOptions = std::vector< std::string >() ) const
	{
		static const size_t MAX_LINE_LENGTH = 100;
		static const size_t THRESHOLD_HELP_OPTION_LENGTH = 24;
		static const std::string HELP_OPTION_PADDING( THRESHOLD_HELP_OPTION_LENGTH, ' ' );

		const char* applicationName = strrchr( application, '/' );
		applicationName = ( nullptr == applicationName ) ? application : applicationName + 1;

		// Usage: $0 {optionsFlags}
		std::string usageIndent( 7 + strlen( applicationName ), ' ' );
		fprintf( stderr, "Usage: %s", applicationName );
		size_t usageLinePosition = usageIndent.length();

		for ( const auto& handlerIter : mOptionsHandlerMap )
		{
			std::string optionString( handlerIter.first );

			if ( ArgumentParser::OptionValue::required == handlerIter.second.valueRequired )
			{
				// Required value
				std::string valueName( handlerIter.second.valueName );
				std::replace( valueName.begin(), valueName.end(), ' ', '_' );
				optionString += " " + valueName;
			}
			else if ( ArgumentParser::OptionValue::optional == handlerIter.second.valueRequired )
			{
				// Optional value
				std::string valueName( handlerIter.second.valueName );
				std::replace( valueName.begin(), valueName.end(), ' ', '_' );
				optionString += " [" + valueName + "]";
			}

			// Optional option flag
			if ( not handlerIter.second.requiredOption )
			{
				optionString = "[" + optionString + "]";
			}

			optionString = " " + optionString;

			// Move option to next line
			if ( MAX_LINE_LENGTH < ( usageLinePosition + optionString.length() ) )
			{
				fprintf( stderr, "\n%s", usageIndent.c_str() );
				usageLinePosition = usageIndent.length();
			}

			// Print option flag and update line position
			fprintf( stderr, "%s", optionString.c_str() );
			usageLinePosition += optionString.length();
		}

		fprintf( stderr, "\n" );

		if ( not missingOptions.empty() )
		{
			// Print the mission options error message.
			fprintf( stderr, "Error: Missing Required Option Flags:\n" );
			for ( const auto& optionFlag : missingOptions )
			{
				fprintf( stderr, "    %s\n", optionFlag.c_str() );
			}
		}
		else
		{
			// Print the help message
			fprintf( stderr, "\n%s\n\nOptions:\n", mApplicationDescription.c_str() );
			fprintf( stderr, "    --help              show this help message and exit\n" );

			for ( const auto& handlerIter : mOptionsHandlerMap )
			{
				std::string optionString( "    " + handlerIter.first );

				if ( ArgumentParser::OptionValue::required == handlerIter.second.valueRequired )
				{
					std::string valueName( handlerIter.second.valueName );
					std::replace( valueName.begin(), valueName.end(), ' ', '_' );
					optionString += " " + valueName;
				}
				else if ( ArgumentParser::OptionValue::optional == handlerIter.second.valueRequired )
				{
					std::string valueName( handlerIter.second.valueName );
					std::replace( valueName.begin(), valueName.end(), ' ', '_' );
					optionString += " [" + valueName + "]";
				}

				if ( THRESHOLD_HELP_OPTION_LENGTH <= optionString.length() )
				{
					fprintf( stderr, "%s\n%s%s\n", optionString.c_str(),
						HELP_OPTION_PADDING.c_str(), handlerIter.second.helpString.c_str() );
				}
				else
				{
					fprintf( stderr, "%s%.*s%s\n", optionString.c_str(),
						static_cast< int >( HELP_OPTION_PADDING.length() - optionString.length() ),
						HELP_OPTION_PADDING.c_str(), handlerIter.second.helpString.c_str() );
				}
			}
		}
	}

public:

	/**
	 * This exception class is thrown when there are expected option flags
	 * not found in the provided arguments list and {@see parseArguments()} is
	 * flagged to throw an exception instead of exiting.
	 */
	class MissingRequiredOption : public std::exception
	{
	private:

		friend class ArgumentParser;

		std::string mMessage;

		MissingRequiredOption(
			const std::vector< std::string >& missingOptions )
		{
			mMessage = std::string( "\n\tMissing option arguments:" );
			for ( const auto& option : missingOptions )
			{
				mMessage.append( "\n\t\t" + option );
			}
			mMessage.append( "\n" );
		}

	public:
		/**
		 * A const pointer to the what string.
		 * @return Pointer to the what message.
		 */
		const char* what() const noexcept
		{
			return mMessage.c_str();
		}

		/**
		 * Copy assignment.
		 * @param other Const reference to the MissingRequiredOption object to copy to this instance.
		 */
		MissingRequiredOption& operator=(
			const MissingRequiredOption& other ) noexcept
		{
			mMessage = other.mMessage;
		}
	};

	/**
	 * Default constructor.
	 */
	ArgumentParser(
		const std::string& applicationDescription = std::string() )
	{
		mApplicationDescription = applicationDescription;
	}

	/**
	 * Move constructor.
	 * @param other R-value to the ArgumentParser to move to this instance.
	 */
	ArgumentParser(
		ArgumentParser&& other )
	{
		_moveAssign( std::move( other ) );
	}

	/**
	 * Copy constructor.
	 * @param other Const reference to the ArgumentParser to copy to this instance.
	 */
	ArgumentParser(
		const ArgumentParser& other )
	{
		_copyAssign( other );
	}

	/**
	 * Add an option and handler for the option.
	 * @param optionString Any unique string to be representative of the option argument. The {@param optionString}
	 *                     if not prefixed with a '--' will have it prefixed. Should there be only 1 dash '-' prefixing
	 *                     the string, then a second dash ( '-' ) will be added so that the option is prefixed with 2 dashes ( '--' ).
	 *                     If {@param optionString} is only 2 dashes ( '--' ), then std::invalid_argument will be thrown.
	 *                     Finally, the normalized optionString may not be '--help', or any capitalization variation, as that is reserved.
	 * @param valueName The name of the value. This is the string value to be used when accessing parsed options from
	 *                  the map returned by {@see getParsedOptions()}. This parameter must be set and unique for options that
	 *                  have a required or optional value. This value is ignored for options that do not take any value. [default: ""]
	 * @param required Boolean indicating that this option is required to be present in the command line arguments. [default: false]
	 * @param helpString A help string to be displayed when --help is present in the command line arguments. [default: ""]
	 * @param valueRequired Define if a value is required for the option flag. [default: OptionValue::required]
	 * @param selection Define which value to take, should the option flag appear more than once in the command line arguments. [default: OptionSelection::take_last]
	 * @param callback A pointer to a callback function to call each time the option flag is found. [default: nullptr]
	 * @param defaultValue The default string value to be passed into the callback, should it be present, in the case
	 *                     that an argument value is not either optional, and not present, or not expected. [default: ""]
	 * @throw std::invalid_argument is thrown if the {@param optionString} is empty, equal to "--", or equal to "--help"
	 * @throw std::invalid_argument is thrown if the {@param optionString} is already defined with a handler.
	 * @throw std::invalid_argument is thrown if the {@param valueName} is already defined.
	 */
	void addOption(
		const std::string& optionString,
		const std::string& valueName = std::string(),
		bool required = false,
		const std::string& helpString = std::string(),
		ArgumentParser::OptionValue valueRequired = ArgumentParser::OptionValue::required,
		ArgumentParser::OptionSelection selection = ArgumentParser::OptionSelection::take_last,
		std::function< void( const std::string& ) > callback = nullptr,
		const std::string& defaultValue = std::string() )
	{
		// TODO: Check that the valueName and optionStrings to not collide.

		std::string normalizedOptionString;

		if ( optionString.empty() )
		{
			throw std::invalid_argument( "Option string may not be empty" );
		}

		// Normalize the optionString, that is: make sure it starts with "--"
		if ( '-' != optionString[ 0 ] )
		{
			normalizedOptionString = "--" + optionString;
		}
		else if ( ( 2 <= optionString.length() ) and ( '-' != optionString[ 1 ] ) )
		{
			normalizedOptionString = "-" + optionString;
		}
		else if ( 3 <= optionString.length() )
		{
			normalizedOptionString = optionString;
		}
		else
		{
			throw std::invalid_argument( "Option string must have more than just \"--\"" );
		}

		// Check that the normalized option string does not equal "--help", ignoring case.
		if ( 0 == strcasecmp( "--help", normalizedOptionString.c_str() ) )
		{
			throw std::invalid_argument( "The normalized option string may not be \"--help\"" );
		}

		// For required and optional values, make sure that the valueName isn't already taken,
		// nor that its valueName collides with an option flag that doesn't take any values.
		if ( ( ArgumentParser::OptionValue::required == valueRequired )
			or ( ArgumentParser::OptionValue::optional == valueRequired ) )
		{
			// Check for empty
			if ( valueName.empty() )
			{
				throw std::invalid_argument( "The valueName may not be the empty string for option flags with an optional or required value" );
			}

			// Check that valueName isn't already taken
			if ( mOptionsValueNames.end() != mOptionsValueNames.find( valueName ) )
			{
				throw std::invalid_argument( "The given valueName \"" + valueName + "\" has already been claimed" );
			}

			// Check that valueName doesn't collide with an option flag that takes no values.
			auto mapIterator = mOptionsHandlerMap.find( valueName );

			if ( mOptionsHandlerMap.end() != mapIterator )
			{
				const auto& optionHandler = mapIterator->second;

				if ( ArgumentParser::OptionValue::none == optionHandler.valueRequired )
				{
					throw std::invalid_argument( "The given valueName \"" + valueName + "\" collides with the no_value option flag: " + mapIterator->first );
				}
			}
		}
		else
		{
			// Check that the option flag doesn't collide with a claimed valueName
			if ( mOptionsValueNames.end() != mOptionsValueNames.find( normalizedOptionString ) )
			{
				throw std::invalid_argument( "The given option flag \"" + normalizedOptionString + "\" collides with the valueName: " + normalizedOptionString );
			}
		}

		// Check that we don't already have a handler for the option flag
		if ( mOptionsHandlerMap.end() != mOptionsHandlerMap.find( normalizedOptionString ) )
		{
			throw std::invalid_argument( "The handler for option \"" + normalizedOptionString + "\" is already defined" );
		}

		// Create the option handler
		_OptionHandler handler;
		handler.defaultStringValue = defaultValue;
		if ( ( ArgumentParser::OptionValue::required == valueRequired )
			or ( ArgumentParser::OptionValue::optional == valueRequired ) )
		{
			handler.valueName = valueName;
			mOptionsValueNames.insert( valueName );
		}
		handler.callback = callback;
		handler.valueRequired = valueRequired;
		handler.selection = selection;
		handler.helpString = helpString;
		handler.requiredOption = required;

		// Add the option handler to the map
		mOptionsHandlerMap[ normalizedOptionString ] = std::move( handler );

		// If required, add the option to the required options map
		if ( required )
		{
			mRequiredOptions[ normalizedOptionString ] = false;
		}
	}

	/**
	 * Clear out the parsed option arguments and non-option arguments.
	 */
	void clear()
	{
		for ( const auto& requiredOption : mRequiredOptions )
		{
			mRequiredOptions[ requiredOption.first ] = false;
		}

		mParsedOptions.clear();
		mNonOptionArguments.clear();
	}

	/**
	 * Get the options parsed from the command line.
	 * For options that do not expect a value, they can retrieved from the
	 * map with their normalized optionString value. Options that have an
	 * optional or required value will need to access their values via the
	 * valueName as set in the addOption() call.
	 * @return A const reference to the parsed options map.
	 */
	const std::map< std::string, OptionArgument >& getParsedOptions() const
	{
		return mParsedOptions;
	}

	/**
	 * Get the vector of non-option arguments parsed.
	 * @return A const reference to the non-option arguments vector.
	 */
	const std::vector< std::string >& getNonOptionArguments() const
	{
		return mNonOptionArguments;
	}

	/**
	 * Check if an option flag has been parsed, or if an associated
	 * valueName for an option flag is present in the parsed options map.
	 * If an option flag is provided that has an associated valueName, then the
	 * valueName is looked up, and the return value is dependent upon the presence of the associated valueName.
	 * @param optionOrValueName Const reference to the option flag, or valueName to check for in the parsed options map.
	 * @return True is returned if the option flag has been parsed, or if the valueName is present.
	 */
	bool hasParsedOption(
		const std::string& optionOrValueName ) const
	{
		if ( 0 == strncmp( optionOrValueName.c_str(), "--", 2 ) )
		{
			auto mapIterator = mOptionsHandlerMap.find( optionOrValueName );

			if ( mOptionsHandlerMap.end() == mapIterator )
			{
				return false;
			}

			const auto& handler = mapIterator->second;

			if ( handler.valueName.empty() )
			{
				return mParsedOptions.end() != mParsedOptions.find( optionOrValueName );
			}
			else
			{
				return mParsedOptions.end() != mParsedOptions.find( handler.valueName );
			}
		}
		else
		{
			return mParsedOptions.end() != mParsedOptions.find( optionOrValueName );
		}
	}

	/**
	 * Move assignment operator.
	 * @param other R-value to the ArgumentParser to move to this instance.
	 * @return Reference to this ArgumentParser instance.
	 */
	ArgumentParser& operator=(
		ArgumentParser&& other )
	{
		if ( this != &other )
		{
			_moveAssign( std::move( other ) );
		}

		return *this;
	}

	/**
	 * Copy assignment operator.
	 * @param other Const reference to the ArgumentParser to copy to this instance.
	 * @return Reference to this ArgumentParser instance.
	 */
	ArgumentParser& operator=(
		const ArgumentParser& other )
	{
		if ( this != &other )
		{
			_copyAssign( other );
		}

		return *this;
	}

	/**
	 * Parse arguments from the c-string array.
	 * If the '--help' option is present, then the
	 * help message is printed and exit() is called.
	 * Should there be any missing required arguments,
	 * then the help message is printed along with which
	 * options were missing, exit() is called after.
	 * @param argc The number of elements in the c-string array.
	 * @param argv An array of c-strings. The array is expected to be null terminated.
	 *             That is, the element at argv[ argc ] is expected to be a null pointer.
	 * @param throwOnMissingOptions Flag that an exception should be thrown
	 *                              instead of calling exit(). [default: false]
	 */
	void parseArguments(
		int argc,
		char const* const* argv,
		bool throwOnMissingOptions = false )
	{
		if ( nullptr != argv[ argc ] )
		{
			throw std::invalid_argument( "The last argument must be NULL" );
		}

		bool helpOptionPresent = false;
		std::vector< std::string > missingOptions;

		// Iterate over arguments
		for ( int index( 0 ); ++index < argc; )
		{
			if ( nullptr == argv[ index ] )
			{
				goto clearAndThrowInvalidArgument;
			}

			std::string argument( argv[ index ] );
			if ( 0 == strncmp( argv[ index ], "--", 2 ) )
			{
				// Check for '--help' before anything else
				if ( 0 == strcasecmp( "--help", argv[ index ] ) )
				{
					_printHelp( argv[ 0 ] );
					exit( EXIT_SUCCESS );
				}

				// Check if the option has a handler
				auto mapIterator = mOptionsHandlerMap.find( argument );

				if ( mOptionsHandlerMap.end() == mapIterator )
				{
					// Output an error message, then ignore
					fprintf( stderr, "Unknown option flag: %s\n", argv[ index ] );
					continue;
				}
				else
				{
					const _OptionHandler& handler = mapIterator->second;
					std::string optionValue( handler.defaultStringValue );

					// Get the value if applicable
					if ( ArgumentParser::OptionValue::optional == handler.valueRequired )
					{
						if ( ( nullptr != argv[ index + 1 ] )
							and ( 0 != strncmp( argv[ index + 1 ], "--", 2 ) ) )
						{
							optionValue.assign( argv[ ++index ] );
						}
					}
					else if ( ArgumentParser::OptionValue::required == handler.valueRequired )
					{
						if ( nullptr == argv[ index + 1 ] )
						{
							fprintf( stderr, "Required value not present for option: %s\n", argument.c_str() );
							continue;
						}

						optionValue.assign( argv[ ++index ] );
					}

					if ( ( ArgumentParser::OptionValue::optional == handler.valueRequired )
						or ( ArgumentParser::OptionValue::required == handler.valueRequired ) )
					{
						// Check how to handle the value
						// Regardless of which value is selected, if nothing is present we insert the first
						if ( mParsedOptions.end() == mParsedOptions.find( handler.valueName ) )
						{
							mParsedOptions.insert(
								{
									handler.valueName,
									OptionArgument( argument, handler.valueName,
										std::vector< std::string > { optionValue } )
								} );
						}
						else
						{
							// Take only the last value
							if ( ArgumentParser::OptionSelection::take_last == handler.selection )
							{
								mParsedOptions[ handler.valueName ].mOptionValues[ 0 ] = optionValue;
							}

							// Push it to the vector, we're taking all the values
							if ( ArgumentParser::OptionSelection::take_all == handler.selection )
							{
								mParsedOptions[ handler.valueName ].mOptionValues.push_back( optionValue );
							}
						}
					}
					else
					{
						mParsedOptions[ argument ] = OptionArgument( argument );
					}

					// Check for a callback
					if ( nullptr != handler.callback )
					{
						handler.callback( optionValue );
					}
				}

				// Check if this is a required option flag
				if ( mRequiredOptions.end() != mRequiredOptions.find( argument ) )
				{
					mRequiredOptions[ argument ] = true;
				}
			}
			else
			{
				mNonOptionArguments.push_back( std::move( argument ) );
			}
		}

		// Check for missing required arguments
		for ( const auto& requiredOption : mRequiredOptions )
		{
			if ( not requiredOption.second )
			{
				missingOptions.push_back( requiredOption.first );
			}
		}

		// Throw if we have missing required arguments
		if ( not missingOptions.empty() )
		{
			if ( throwOnMissingOptions )
			{
				throw MissingRequiredOption( missingOptions );
			}

			_printHelp( argv[ 0 ], missingOptions );
			exit( EXIT_FAILURE );
		}

		return;

	clearAndThrowInvalidArgument:
		mParsedOptions.clear();
		mNonOptionArguments.clear();
		for ( const auto& requiredOption : mRequiredOptions )
		{
			mRequiredOptions[ requiredOption.first ] = false;
		}

		throw std::invalid_argument( "Null pointer found in the middle of the arguments list." );
	}

	/**
	 * Set the application description.
	 * @param applicationDescription The description to what this application is for.
	 */
	void setApplicationDescription(
		const std::string& applicationDescription )
	{
		mApplicationDescription = applicationDescription;
	}
};
