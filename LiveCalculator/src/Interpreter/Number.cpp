#include "Pch.h"

#include "Number.h"

Number::Number(const int64_t Value)
	: IsInt(true), IntValue(Value), LongDoubleValue(0)
{
}

Number::Number(const long double Value)
	: IsInt(false), IntValue(0), LongDoubleValue(Value)
{
}

[[nodiscard]] Number Number::AddedTo(const Number& Other) const
{
	if (IsInt && Other.IsInt)
	{
		return Number(IntValue + Other.IntValue);
	}

	if (IsInt && !Other.IsInt)
	{
		return Number(static_cast<long double>(IntValue) + Other.LongDoubleValue);
	}

	if (!IsInt && Other.IsInt)
	{
		return Number(LongDoubleValue + static_cast<long double>(Other.IntValue));
	}

	return Number(LongDoubleValue + Other.LongDoubleValue);
}

[[nodiscard]] Number Number::SubtractedBy(const Number& Other) const
{
	if (IsInt && Other.IsInt)
	{
		return Number(IntValue - Other.IntValue);
	}

	if (IsInt && !Other.IsInt)
	{
		return Number(static_cast<long double>(IntValue) - Other.LongDoubleValue);
	}

	if (!IsInt && Other.IsInt)
	{
		return Number(LongDoubleValue - static_cast<long double>(Other.IntValue));
	}

	return Number(LongDoubleValue - Other.LongDoubleValue);
}

[[nodiscard]] Number Number::MultipliedBy(const Number& Other) const
{
	if (IsInt && Other.IsInt)
	{
		return Number(IntValue * Other.IntValue);
	}

	if (IsInt && !Other.IsInt)
	{
		return Number(static_cast<long double>(IntValue) * Other.LongDoubleValue);
	}

	if (!IsInt && Other.IsInt)
	{
		return Number(LongDoubleValue * static_cast<long double>(Other.IntValue));
	}

	return Number(LongDoubleValue * Other.LongDoubleValue);
}

[[nodiscard]] Number Number::DividedBy(const Number& Other) const
{
	if (IsInt && Other.IsInt)
	{
		return Number(static_cast<long double>(IntValue) / static_cast<long double>(Other.IntValue));
	}

	if (IsInt && !Other.IsInt)
	{
		return Number(static_cast<long double>(IntValue) / Other.LongDoubleValue);
	}

	if (!IsInt && Other.IsInt)
	{
		return Number(LongDoubleValue / static_cast<long double>(Other.IntValue));
	}

	return Number(LongDoubleValue / Other.LongDoubleValue);
}
