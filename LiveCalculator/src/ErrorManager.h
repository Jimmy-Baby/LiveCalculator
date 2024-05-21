#pragma once

#include "Printable.h"
#include "Position.h"

class Error
{
public:
	Error() // Constructor for no error
		: Error("", "", Position(0, 0, 0, ""), Position(0, 0, 0, ""))
	{
	}

	explicit Error(std::string ErrorName,
	               std::string Details,
	               Position PosStart,
	               Position PosEnd)
		: ErrorName(std::move(ErrorName)),
		  Details(std::move(Details)),
		  Start(std::move(PosStart)),
		  End(std::move(PosEnd))
	{
	}

	[[nodiscard]] std::string StringWithArrows(const std::string& Text) const;

	[[nodiscard]] Error* GetError()
	{
		return this;
	}

	std::string ErrorName;
	std::string Details;

	// Protected fields and functions
protected:
	Position Start;
	Position End;
};

class ErrorManager
{
	// Access functions
public:
	// Return the stored error object if an error was stored, returns nullptr if no error.
	[[nodiscard]] static Error* GetLastError();

	// Returns true if no error is found from the last operation.
	static bool CheckLastError();

	// Set error info
	static Error* SetLastError(Error InError)
	{
		*LastError = std::move(InError);
		return LastError.get();
	}

	// Clear all error info
	static void Clear()
	{
		LastError->ErrorName.clear();
		LastError->Details.clear();
	}

	// Protected fields and functions
protected:
	static std::unique_ptr<Error> LastError;
};
