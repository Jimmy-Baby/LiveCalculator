#pragma once

#include "../Parser/NodeTypes.h"
#include "Number.h"

namespace Interpreter
{
	Number Visit(NodeBase* Root, bool ClearError = false);
	Number VisitNumberNode(const NumberNode* Node);
	Number VisitBinaryOperator(const BinaryOpNode* Node);
	Number VisitUnaryOperator(const UnaryOpNode* Node);
}
