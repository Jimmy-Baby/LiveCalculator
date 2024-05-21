#pragma once

#include "Token.h"
#include "Position.h"

class Lexer
{
public:
	static std::vector<Token> GetTokens(std::string& Input);
	static void Advance();

	[[nodiscard]] static Token GetNumberToken();
	[[nodiscard]] static auto GetInput() { return CurrentInput; }
	[[nodiscard]] static auto GetCurrentPosition() { return CurrentPosition; }
	[[nodiscard]] static auto GetCurrentCharacter() { return CurrentCharacter; }

private:
	static std::string* CurrentInput;
	static Position CurrentPosition;
	static char CurrentCharacter;
};
