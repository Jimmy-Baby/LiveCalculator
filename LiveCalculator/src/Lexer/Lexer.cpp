// Precompiled headers
#include "Pch.h"

#include "Lexer.h"
#include "ErrorManager.h"

std::string* Lexer::CurrentInput = nullptr;
Position Lexer::CurrentPosition(-1, 0, -1, "");
char Lexer::CurrentCharacter = '\0';

std::vector<Token> Lexer::GetTokens(std::string& Input)
{
	ErrorManager::Clear();

	std::vector<Token> Result;

	CurrentInput = &Input;
	CurrentPosition = Position(-1, 0, -1, Input);
	CurrentCharacter = '\0';

	Advance();

	while (CurrentCharacter != '\0')
	{
		if (CurrentCharacter == ' ' || CurrentCharacter == '\t')
		{
			Advance();
		}
		else if (isdigit(CurrentCharacter))
		{
			Result.emplace_back(GetNumberToken());
		}
		else if (CurrentCharacter == '+')
		{
			Result.emplace_back(TYPE_PLUS, "", CurrentPosition);
			Advance();
		}
		else if (CurrentCharacter == '-')
		{
			Result.emplace_back(TYPE_MINUS, "", CurrentPosition);
			Advance();
		}
		else if (CurrentCharacter == '*')
		{
			Result.emplace_back(TYPE_MUL, "", CurrentPosition);
			Advance();
		}
		else if (CurrentCharacter == '/')
		{
			Result.emplace_back(TYPE_DIV, "", CurrentPosition);
			Advance();
		}
		else if (CurrentCharacter == '(')
		{
			Result.emplace_back(TYPE_LBRACKET, "", CurrentPosition);
			Advance();
		}
		else if (CurrentCharacter == ')')
		{
			Result.emplace_back(TYPE_RBRACKET, "", CurrentPosition);
			Advance();
		}
		else
		{
			const std::string ErrorStr = std::format("'{}'", CurrentCharacter);
			const Position ErrorStartPosition = CurrentPosition;

			// Advance so we can recapture m_Pos after it has moved forward
			Advance();

			ErrorManager::SetLastError(Error("Illegal Character", ErrorStr, ErrorStartPosition, CurrentPosition));
			return {};
		}
	}

	Result.emplace_back(TYPE_EOF, "", CurrentPosition);
	return Result;
}

void Lexer::Advance()
{
	CurrentPosition.Advance(CurrentCharacter);

	if (CurrentPosition.Index < static_cast<int32_t>(CurrentInput->size()))
	{
		CurrentCharacter = CurrentInput->at(CurrentPosition.Index);
	}
	else
	{
		CurrentCharacter = '\0';
	}
}

[[nodiscard]] Token Lexer::GetNumberToken()
{
	std::string NumberString;
	int32_t PeriodCount = 0;

	Position StartPosition = CurrentPosition;

	while (CurrentCharacter != '\0' && (isdigit(CurrentCharacter) || CurrentCharacter == '.'))
	{
		if (CurrentCharacter == '.')
		{
			if (PeriodCount == 1)
			{
				break;
			}

			PeriodCount++;
			NumberString += '.';
		}
		else
		{
			NumberString += CurrentCharacter;
		}

		Advance();
	}

	if (PeriodCount == 0)
	{
		return { TYPE_INT, NumberString, std::move(StartPosition), CurrentPosition };
	}

	return { TYPE_FLOAT, NumberString, std::move(StartPosition), CurrentPosition };
}
