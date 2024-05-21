#pragma once

#include "../Lexer/Token.h"

enum ENodeType
{
	NODE_TYPE_NUMBER,
	NODE_TYPE_BINARY_OP,
	NODE_TYPE_UNARY_OP
};

class NodeBase : public Printable
{
public:
	explicit NodeBase(const ENodeType InType, Position StartPosition, Position EndPosition)
		: Type(InType),
		  Start(std::move(StartPosition)),
		  End(std::move(EndPosition))
	{
	}

	[[nodiscard]] virtual bool IsValid() const = 0;
	[[nodiscard]] virtual std::string GetPrintableTokenString() const = 0;

	ENodeType Type;
	Position Start;
	Position End;
};

class NumberNode final : public NodeBase
{
public:
	explicit NumberNode(Token* InToken)
		: NodeBase(NODE_TYPE_NUMBER, InToken->Start, InToken->End),
		  Token(InToken)
	{
		IsInt = InToken->Type == TYPE_INT ? true : false;
	}

	[[nodiscard]] bool IsValid() const override
	{
		return Token != nullptr;
	}

	[[nodiscard]] std::string GetPrintableTokenString() const override
	{
		if (Token == nullptr)
		{
			return {};
		}

		return Token->GetPrintableTokenString();
	}

	void Print() override
	{
		printf("%s", GetPrintableTokenString().c_str());
	}

	[[nodiscard]] Token* GetToken() const
	{
		return Token;
	}

	[[nodiscard]] std::string& GetValue() const
	{
		return Token->Value;
	}

	[[nodiscard]] int64_t GetIntValue() const
	{
		return std::strtoll(GetValue().c_str(), nullptr, 10);
	}

	[[nodiscard]] long double GetLongDoubleValue() const
	{
		return std::strtold(GetValue().c_str(), nullptr);
	}

	bool IsInt;

	// Protected fields and functions
protected:
	Token* Token;
};

class BinaryOpNode final : public NodeBase
{
public:
	explicit BinaryOpNode(Token* Token, NodeBase* Left, NodeBase* Right)
		: NodeBase(NODE_TYPE_BINARY_OP, Left->Start, Right->End),
		  Token(Token),
		  LeftNode(Left),
		  RightNode(Right)
	{
	}

	[[nodiscard]] bool IsValid() const override
	{
		if (Token != nullptr &&
			LeftNode->IsValid() &&
			RightNode->IsValid())
		{
			return true;
		}

		return false;
	}

	[[nodiscard]] std::string GetPrintableTokenString() const override
	{
		std::string LeftTokenString;
		std::string MyTokenString;
		std::string RightTokenString;

		if (LeftNode)
		{
			LeftTokenString = LeftNode->GetPrintableTokenString();
		}

		if (Token)
		{
			MyTokenString = Token->GetPrintableTokenString();
		}

		if (RightNode)
		{
			RightTokenString = RightNode->GetPrintableTokenString();
		}

		return std::format("({}, {}, {})", LeftTokenString, MyTokenString, RightTokenString);
	}

	void Print() override
	{
		printf("%s", GetPrintableTokenString().c_str());
	}

	Token* Token;
	NodeBase* LeftNode;
	NodeBase* RightNode;
};

class UnaryOpNode final : public NodeBase
{
public:
	explicit UnaryOpNode(Token* OperatorToken, NodeBase* Node)
		: NodeBase(NODE_TYPE_UNARY_OP, OperatorToken->Start, Node->End),
		  OperatorToken(OperatorToken),
		  ChildNode(Node)
	{
	}

	[[nodiscard]] bool IsValid() const override
	{
		return OperatorToken != nullptr && ChildNode != nullptr;
	}

	[[nodiscard]] std::string GetPrintableTokenString() const override
	{
		std::string Token;
		std::string NodeString;

		if (OperatorToken)
		{
			Token = OperatorToken->GetPrintableTokenString();
		}

		if (ChildNode)
		{
			NodeString = ChildNode->GetPrintableTokenString();
		}

		return std::format("({}, {})", Token.c_str(), NodeString.c_str());
	}

	void Print() override
	{
		printf("%s", GetPrintableTokenString().c_str());
	}

	Token* OperatorToken;
	NodeBase* ChildNode;
};
