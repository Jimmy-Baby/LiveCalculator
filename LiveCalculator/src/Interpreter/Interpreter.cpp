#include "Pch.h"

#include "Interpreter.h"
#include "ErrorManager.h"

namespace Interpreter
{
	Number Visit(NodeBase* Root, const bool ClearError)
	{
		if (ClearError)
		{
			ErrorManager::Clear();
		}

		switch (Root->Type)
		{
		case NODE_TYPE_BINARY_OP:
			return VisitBinaryOperator(dynamic_cast<BinaryOpNode*>(Root));

		case NODE_TYPE_NUMBER:
			return VisitNumberNode(dynamic_cast<NumberNode*>(Root));

		case NODE_TYPE_UNARY_OP:
			return VisitUnaryOperator(dynamic_cast<UnaryOpNode*>(Root));
		}

		return Number(0i64);
	}

	Number VisitNumberNode(const NumberNode* Node)
	{
		if (Node->IsInt)
		{
			return Number(Node->GetIntValue());
		}

		return Number(Node->GetLongDoubleValue());
	}

	Number VisitBinaryOperator(const BinaryOpNode* Node)
	{
		const Number Left = Visit(Node->LeftNode);

		if (!ErrorManager::CheckLastError())
		{
			return Number(0i64);
		}

		const Number Right = Visit(Node->RightNode);

		if (!ErrorManager::CheckLastError())
		{
			return Number(0i64);
		}

		switch (Node->Token->Type)
		{
		case TYPE_PLUS:
			return Left.AddedTo(Right);

		case TYPE_MINUS:
			return Left.SubtractedBy(Right);

		case TYPE_MUL:
			return Left.MultipliedBy(Right);

		case TYPE_DIV:
			if (Right.IsInt && Right.IntValue == 0 || !Right.IsInt && Right.LongDoubleValue == 0.0)
			{
				ErrorManager::SetLastError(Error("Runtime Error", "Integer Division by 0", Node->Start, Node->End));
				return Number(0i64);
			}

			return Left.DividedBy(Right);

		case TYPE_FLOAT:
		case TYPE_LBRACKET:
		case TYPE_RBRACKET:
		case TYPE_EOF:
		case TYPE_INT:
			break;
		}

		return Number(0i64);
	}

	Number VisitUnaryOperator(const UnaryOpNode* Node)
	{
		const Number Child = Visit(Node->ChildNode);

		if (!ErrorManager::CheckLastError())
		{
			return Number(0i64);
		}

		if (Node->OperatorToken->Type == TYPE_MINUS)
		{
			return Child.MultipliedBy(Number(-1i64));
		}

		return Number(0i64);
	}
}
