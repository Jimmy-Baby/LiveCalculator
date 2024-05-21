#pragma once

#include "NodeTypes.h"

class Parser
{
public:
	static NodeBase* GetExpressionResult(const std::vector<Token>& InTokens);
	static Token* Advance();
	[[nodiscard]] static NodeBase* GetFactor();
	[[nodiscard]] static NodeBase* GetTerm();
	[[nodiscard]] static NodeBase* GetExpression();

	template <class NodeTy, class... Args>
	[[nodiscard]] static NodeTy* CreateNode(Args... NodeArgs)
	{
		// Create new node
		Nodes.emplace_back(std::make_unique<NodeTy>(NodeArgs...));

		// Get raw ptr to node and return
		return static_cast<NodeTy*>(Nodes.back().get());
	}

	template <class NodeTy, class RetTy, class... Args>
	[[nodiscard]] static RetTy* CreateNode(Args... NodeArgs)
	{
		// Create new node
		Nodes.emplace_back(std::make_unique<NodeTy>(NodeArgs...));

		// Get raw ptr to node and return
		return static_cast<RetTy*>(Nodes.back().get());
	}

	// Protected fields and functions
protected:
	static std::vector<Token> Tokens;
	static std::vector<std::unique_ptr<NodeBase>> Nodes;
	static Token* CurrentToken;
	static int32_t TokenIndex;
};
