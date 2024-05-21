// Precompiled headers
#include "Pch.h"

#include "Parser.h"
#include "ErrorManager.h"

/*
 * expr: term ((PLUS | MINUS) term)*
 * term: factor ((MUL | DIV) factor)*
 * factor: INT | FLOAT
 *		   (PLUS|MINUS) factor
 *		   LBRACKET expr RBRACKET
 */

std::vector<Token> Parser::Tokens;
std::vector<std::unique_ptr<NodeBase>> Parser::Nodes;
Token* Parser::CurrentToken;
int32_t Parser::TokenIndex;

NodeBase* Parser::GetExpressionResult(const std::vector<Token>& InTokens)
{
	ErrorManager::Clear();

	Tokens = InTokens;
	CurrentToken = nullptr;
	TokenIndex = -1;

	Advance();

	NodeBase* Result = GetExpression();

	// Checks for errors from parsing
	if (!ErrorManager::CheckLastError())
	{
		return Result;
	}
	 
	// Check that we actually reached end of the file/string
	// Otherwise there was an error at some point
	if (CurrentToken->Type != TYPE_EOF)
	{
		ErrorManager::SetLastError(Error("Invalid Syntax", "Expected operator ('+', '-', '*', '/')", CurrentToken->Start, CurrentToken->End));
		return Result;
	}

	return Result;
}

Token* Parser::Advance()
{
	TokenIndex++;

	if (TokenIndex < static_cast<int32_t>(Tokens.size()))
	{
		CurrentToken = &Tokens.at(TokenIndex);
	}

	return CurrentToken;
}

[[nodiscard]] NodeBase* Parser::GetFactor()
{
	Token* SavedToken = CurrentToken;

	if (SavedToken->Type == TYPE_PLUS || SavedToken->Type == TYPE_MINUS)
	{
		Advance();
		NodeBase* Factor = GetFactor();

		if (!ErrorManager::CheckLastError())
		{
			return nullptr;
		}

		return CreateNode<UnaryOpNode>(SavedToken, Factor);
	}

	if (SavedToken->Type == TYPE_INT || SavedToken->Type == TYPE_FLOAT)
	{
		Advance();
		return CreateNode<NumberNode>(SavedToken);
	}

	if (SavedToken->Type == TYPE_LBRACKET)
	{
		Advance();
		NodeBase* Expression = GetExpression();

		if (!ErrorManager::CheckLastError())
		{
			return nullptr;
		}

		if (CurrentToken->Type == TYPE_RBRACKET)
		{
			Advance();
			return Expression;
		}

		ErrorManager::SetLastError(Error("Invalid Syntax", "Expected ')'", CurrentToken->Start, CurrentToken->End));

		return nullptr;
	}

	ErrorManager::SetLastError(Error("Invalid Syntax", "Expected integer or floating-point number", SavedToken->Start, SavedToken->End));

	return nullptr;
}

[[nodiscard]] NodeBase* Parser::GetTerm()
{
	NodeBase* LeftToken = GetFactor();

	if (!ErrorManager::CheckLastError())
	{
		return nullptr;
	}

	while (CurrentToken->Type == TYPE_MUL || CurrentToken->Type == TYPE_DIV)
	{
		Token* OperatorToken = CurrentToken;

		Advance();

		const auto RightToken = GetFactor();

		if (!ErrorManager::CheckLastError())
		{
			return nullptr;
		}

		LeftToken = static_cast<NodeBase*>(CreateNode<BinaryOpNode>(OperatorToken, LeftToken, RightToken));
	}

	return LeftToken;
}

[[nodiscard]] NodeBase* Parser::GetExpression()
{
	auto LeftToken = GetTerm();

	if (!ErrorManager::CheckLastError())
	{
		return nullptr;
	}

	while (CurrentToken->Type == TYPE_PLUS || CurrentToken->Type == TYPE_MINUS)
	{
		Token* OperatorToken = CurrentToken;

		Advance();

		const auto RightToken = GetTerm();

		if (!ErrorManager::CheckLastError())
		{
			return nullptr;
		}

		LeftToken = static_cast<NodeBase*>(CreateNode<BinaryOpNode>(OperatorToken, LeftToken, RightToken));
	}

	return LeftToken;
}
