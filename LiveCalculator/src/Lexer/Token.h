#pragma once
#pragma warning (disable : 26812)

#include "Position.h"
#include "Printable.h"

enum ETokenType
{
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_PLUS,
	TYPE_MINUS,
	TYPE_MUL,
	TYPE_DIV,
	TYPE_LBRACKET,
	TYPE_RBRACKET,
	TYPE_EOF
};

inline const char* GTokenTypeNames[] =
{
	"INT",
	"FLOAT",
	"PLUS",
	"MINUS",
	"MUL",
	"DIV",
	"LBRACKET",
	"RBRACKET",
	"TYPE_EOF"
};

class Token final : public Printable
{
public:
	Token() = delete;

	Token(ETokenType Type, std::string Value = "", Position StartPos = { -1, 0, 0, "" }, Position EndPos = { -1, 0, 0, "" });
	[[nodiscard]] std::string GetPrintableTokenString() const;
	void Print() override;

	ETokenType Type;
	std::string Value;
	Position Start;
	Position End;
};
