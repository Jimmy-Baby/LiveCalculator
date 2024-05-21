// Precompiled headers
#include "Pch.h"

#include "ErrorManager.h"

std::unique_ptr<Error> ErrorManager::LastError = std::make_unique<Error>();

[[nodiscard]] std::string Error::StringWithArrows(const std::string& Text) const
{
	std::string ReturnValue;

	uint64_t IndexStart = std::max(Text.rfind('\n', Start.Index), 0ui64);
	uint64_t IndexEnd = Text.find('\n', IndexStart + 1);

	if (IndexStart == std::string::npos)
	{
		IndexStart = 0;
	}

	if (IndexEnd == std::string::npos)
	{
		IndexEnd = Text.length();
	}

	const int LineCount = End.LineNumber - Start.LineNumber + 1;

	for (int LineIndex = 0; LineIndex < LineCount; ++LineIndex)
	{
		std::string Line = Text.substr(IndexStart, IndexEnd - IndexStart);

		int ColumnStart;
		int ColumnEnd;

		if (LineIndex == 0)
		{
			ColumnStart = Start.ColumnNumber;
		}
		else
		{
			ColumnStart = 0;
		}

		if (LineIndex == LineCount - 1)
		{
			ColumnEnd = End.ColumnNumber;
		}
		else
		{
			ColumnEnd = static_cast<int>(Line.length()) - 1;
		}

		ReturnValue += Line + '\n';

		// Add out whitespace
		for (int ColumnIndex = 0; ColumnIndex < ColumnStart; ++ColumnIndex)
		{
			ReturnValue += ' ';
		}

		// Add our arrows
		for (int ColumnIndex = 0; ColumnIndex < ColumnEnd - ColumnStart; ++ColumnIndex)
		{
			ReturnValue += '^';
		}

		// Recalculate indexes
		IndexStart = IndexEnd;
		IndexEnd = Text.find('\n', IndexStart + 1);

		if (IndexEnd == std::string::npos)
		{
			IndexEnd = Text.length();
		}
	}

	std::erase(ReturnValue, '\t');

	return ReturnValue;
}

[[nodiscard]] Error* ErrorManager::GetLastError()
{
	return LastError.get();
}

bool ErrorManager::CheckLastError()
{
	return LastError->ErrorName.empty();
}
