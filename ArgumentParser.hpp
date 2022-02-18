/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class OptionArgument
{
private:

	friend class ArgumentParser;

	std::string mOptionString;
	std::vector< std::string > mOptionValues;

	// move assign
	void _moveAssign(
		OptionArgument&& other )
	{
		mOptionString = std::move( other.mOptionString );
		mOptionValues = std::move( other.mOptionValues );
	}

	// copy assign
	void _copyAssign(
		const OptionArgument& other )
	{
		mOptionString = other.mOptionString;
		mOptionValues = other.mOptionValues;
	}

	// assignment constructor; this'll be called by ArgumentParser
	OptionArgument(
		const std::string& optionString,
		const std::vector< std::string >& optionValues )
	{
		mOptionString = optionString;
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
	 * Get the value at the given index cast as the provided type.
	 * The casting will be from std::string. For arithmetic types, the STL
	 * funtions std::sto{ld,ull,ll} are used, downcasting shall occur for
	 * artihmetic types shorter than the max length. For all other types
	 * the std::string value shall be passed to a constructor.
	 * @param index The index of the value to be cast from a string. [default: 0]
	 * @return The value at the requested index cast to the desired Type.
	 * @throw std::out_of_range is thrown if the index is out of range.
	 */
	template< typename Type >
	Type valueAs( size_t index = 0 ) const
	{
		// This branch can be nipped for non-arithmetic
		// types by turning on the optimizer.
		if ( std::is_arithmetic< Type >::value )
		{
			const char* valueCString = mOptionValues.at( index ).c_str();
			if ( std::is_floating_point< Type >::value )
			{
				long double longDoubleValue = std::strtold( valueCString, nullptr );
				return static_cast< Type >( longDoubleValue );
			}

			if ( std::is_unsigned< Type >::value )
			{
				static const Type unsignedTypeMax = ~( ( Type ) 0 );

				unsigned long long int value = std::strtoull( valueCString, nullptr, 0 );
				return ( value > unsignedTypeMax ) ? unsignedTypeMax : static_cast< Type >( value );
			}

			if ( std::is_signed< Type >::value )
			{
				static const Type signedTypeMin = ( ( Type ) 1 ) << ( 8 * sizeof( Type ) - 1 );
				static const Type signedTypeMax = ( ~( ( Type ) 0 ) ) >> 1;

				long long int value = std::strtoll( valueCString, nullptr, 0 );

				return ( value < 0 )
					? ( ( value < signedTypeMin ) ? signedTypeMin : static_cast< Type >( value ) )
					: ( ( value > signedTypeMax ) ? signedTypeMax : static_cast< Type >( value ) );
			}
		}

		return Type( mOptionValues.at( index ) );
	}
};

class ArgumentParser
{
public:

	enum class OptionValue : int
	{
		none,
		optional,
		required
	};

	enum class OptionSelection : int
	{
		take_first,
		take_last,
		take_all
	};

private:

	class _OptionHandler
	{
	public:

		std::string defaultStringValue;
		std::function< void( const std::string& ) > callback;
		ArgumentParser::OptionValue valueRequired;
		ArgumentParser::OptionSelection selection;
		std::string helpString;

	private:

		// move assignment
		void _moveAssign(
			_OptionHandler&& other )
		{
			this->defaultStringValue = std::move( other.defaultStringValue );
			this->callback = std::exchange( other.callback, nullptr );
			this->valueRequired = std::exchange( other.valueRequired, ArgumentParser::OptionValue::optional );
			this->selection = std::exchange( other.selection, ArgumentParser::OptionSelection::take_last );
			this->helpString = std::move( other.helpString );
		}

		// copy assignment
		void _copyAssign(
			const _OptionHandler& other )
		{
			this->defaultStringValue = other.defaultStringValue;
			this->callback = other.callback;
			this->valueRequired = other.valueRequired;
			this->selection = other.selection;
			this->helpString = other.helpString;
		}

	public:

		// default constructor
		_OptionHandler()
		{
			this->defaultStringValue = std::string( "" );
			this->callback = nullptr;
			this->valueRequired = ArgumentParser::OptionValue::optional;
			this->selection = ArgumentParser::OptionSelection::take_last;
			this->helpString = std::string( "" );
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

	// Set - Options to be handled
	std::map< std::string, _OptionHandler > mOptionsHandlerMap;

	// Parsed - Options parsed
	std::map< std::string, bool > mRequiredOptions;
	std::map< std::string, OptionArgument > mParsedOptions;
	std::vector< std::string > mNonOptionArguments;

	// Move assignment
	void _moveAssign(
		ArgumentParser&& other )
	{
		mOptionsHandlerMap = std::move( other.mOptionsHandlerMap );
		mRequiredOptions = std::move( other.mRequiredOptions );
		mParsedOptions = std::move( other.mParsedOptions );
		mNonOptionArguments = std::move( other.mNonOptionArguments );
	}

	// Copy assignment
	void _copyAssign(
		const ArgumentParser& other )
	{
		mOptionsHandlerMap = other.mOptionsHandlerMap;
		mRequiredOptions = other.mRequiredOptions;
		mParsedOptions = other.mParsedOptions;
		mNonOptionArguments = other.mNonOptionArguments;
	}

public:

	class MissingRequiredOption : public std::exception
	{
	private:

		friend class ArgumentParser;

		std::string mMessage;

		MissingRequiredOption(
			const std::vector< std::string >& missingOptions )
		{
			mMessage = std::string( "Missing option arguments:" );
			for ( const auto& option : missingOptions )
			{
				mMessage.append( "\n\t" + option );
			}
			mMessage.append( "\n" );
		}

	public:
		const char* what() const noexcept
		{
			return mMessage.c_str();
		}

		MissingRequiredOption& operator=(
			const MissingRequiredOption& other ) noexcept
		{
			mMessage = other.mMessage;
		}
	};

	/**
	 * Default constructor.
	 */
	ArgumentParser()
	{
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
	 * @param optionString
	 * @param required
	 * @param helpString
	 * @param valueRequired
	 * @param selection
	 * @param callback
	 * @param defaultValue The default string value to be passed into the callback, should it be present, in the case
	 *                     that an argument value is not
	 */
	void addOption(
		const std::string& optionString,
		bool required = false,
		const std::string& helpString = std::string(),
		ArgumentParser::OptionValue valueRequired = ArgumentParser::OptionValue::optional,
		ArgumentParser::OptionSelection selection = ArgumentParser::OptionSelection::take_last,
		std::function< void( const std::string& ) > callback = nullptr,
		const std::string& defaultValue = std::string() )
	{
		if ( mOptionsHandlerMap.end() != mOptionsHandlerMap.find( optionString ) )
		{
			throw std::invalid_argument( "The handler for option \"" + optionString + "\" is already defined" );
		}

		_OptionHandler handler;
		handler.defaultStringValue = defaultValue;
		handler.callback = callback;
		handler.valueRequired = valueRequired;
		handler.selection = selection;
		handler.helpString = helpString;

		mOptionsHandlerMap[ optionString ] = std::move( handler );

		if ( required )
		{
			mRequiredOptions[ optionString ] = false;
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
	 * @param argc The number of elements in the c-string array.
	 * @param argv An array of c-strings. The array is expected to be null terminated.
	 *             That is, the element at argv[ argc ] is expected to be a null pointer.
	 */
	void parseArguments(
		int argc,
		char const* const* argv )
	{
		if ( nullptr != argv[ argc ] )
		{
			throw std::invalid_argument( "" );
		}

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
				// Check if the option has a handler
				auto mapIterator = mOptionsHandlerMap.find( argument );

				if ( mOptionsHandlerMap.end() == mapIterator )
				{
					// For the moment, output an error message
					fprintf( stderr, "Unknown option flag: %s\n", argv[ index ] );
					continue;
				}
				else
				{
					_OptionHandler& handler = mapIterator->second;
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
						}

						optionValue.assign( argv[ ++index ] );
					}

					// Check how to handle the value
					// Regardless of which value is selected, if nothing is present we insert the first
					if ( mParsedOptions.end() == mParsedOptions.find( argument ) )
					{
						mParsedOptions.insert(
							{
								argument,
								OptionArgument( argument,
									std::vector< std::string > { optionValue } )
							} );
					}

					switch ( handler.selection )
					{
					case ArgumentParser::OptionSelection::take_first:
						// We don't need to do anything else
						break;

					case ArgumentParser::OptionSelection::take_last:
						mParsedOptions[ argument ].mOptionValues[ 0 ] = optionValue;
						break;

					case ArgumentParser::OptionSelection::take_all:
						mParsedOptions[ argument ].mOptionValues.push_back( optionValue );
						break;
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
			if ( not mRequiredOptions[ requiredOption.first ] )
			{
				missingOptions.push_back( requiredOption.first );
			}
		}

		// Throw if we have missing required arguments
		if ( not missingOptions.empty() )
		{
			throw MissingRequiredOption( missingOptions );
		}

		return;

	clearAndThrowInvalidArgument:
		mParsedOptions.clear();
		mNonOptionArguments.clear();
		for ( const auto& requiredOption : mRequiredOptions )
		{
			mRequiredOptions[ requiredOption.first ] = false;
		}

		throw std::invalid_argument( "" );
	}

	/**
	 * Get the options parsed from the command line.
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
};
