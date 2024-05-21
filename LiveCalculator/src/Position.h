#pragma once

class Position
{
public:
	Position(const int32_t Index, const int32_t LineNumber, const int32_t ColumnNumber, std::string Input)
		: Index(Index),
		  LineNumber(LineNumber),
		  ColumnNumber(ColumnNumber),
		  Input(std::move(Input))
	{
	}

	Position* Advance(const char CurrentCharacter = '\0')
	{
		Index++;
		ColumnNumber++;

		if (CurrentCharacter == '\n')
		{
			LineNumber++;
			ColumnNumber = 0;
		}

		return this;
	}
	
	int32_t Index;
	int32_t LineNumber;
	int32_t ColumnNumber;
	std::string Input;
};
