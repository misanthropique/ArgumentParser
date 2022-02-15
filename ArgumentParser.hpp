/**
 * Copyright ©2022. Brent Weichel. All Rights Reserved.
 * Permission to use, copy, modify, and/or distribute this software, in whole
 * or part by any means, without express prior written agreement is prohibited.
 */
#pragma once

class OptionArgument
{
private:
	std::string mOptionString;
	std::vector< std::string > mOptionValues;

	OptionArgument(
		const std::string& optionString,
		const std::vector< std::string >& optionValues )
	{
	}

public:
	const std::string& optionString() const;

	template< typename Type >
	Type valueAs( size_t index ) const;
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
	private:
		std::string mDefaultStringValue;
		std::function< void( const std::string& ) > mCallback;
		ArgumentParser::OptionValue mValueRequired;
		ArgumentParser::OptionSelection mSelection;
		std::string mHelpString;

		void _moveAssign(
			_OptionHandler&& other )
		{
		}

		void _copyAssign(
			const _OptionHandler& other )
		{
		}

	public:
		_OptionHandler(
			_OptionHandler&& other )
		{
		}

		_OptionHandler(
			const _OptionHandler& other )
		{
		}

		_OptionHandler& operator=(
			_OptionHandler&& other )
		{
		}

		_OptionHandler& operator(
			const _OptionHandler& other )
		{
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
	 * @param defaultValue
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
	ArgumentParser& operator(
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
	 * @param argv An array of c-strings.
	 */
	void parserArguments(
		int argc,
		const char** argv )
	{
		if ( nullptr != argv[ argc ] )
		{
			throw std::invalid_argument( "" );
		}

		// Iterate over
		for ( int index( 0 ); ++index < argc; )
		{
			if ( nullptr == argv[ index ] )
			{
				goto clearAndThrowInvalidArgument;
			}

			if ( '-' == argv[ index ][ 0 ] )
			{
				
			}
		}

		// Check for missing required arguments
		std::vector< std::string > missingOptions;
		for ( const auto& optionString : mRequiredOptions )
		{
			if ( not mRequiredOptions[ optionString ] )
			{
				missingOptions.push_back( optionString );
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
		for ( const auto& optionString : mRequiredOptions )
		{
			mRequiredOptions[ optionString ] = false;
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
