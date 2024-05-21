#pragma once

class Number
{
	// Access functions
public:
	explicit Number(int64_t Value);
	explicit Number(long double Value);

	[[nodiscard]] Number AddedTo(const Number& Other) const;
	[[nodiscard]] Number SubtractedBy(const Number& Other) const;
	[[nodiscard]] Number MultipliedBy(const Number& Other) const;
	[[nodiscard]] Number DividedBy(const Number& Other) const;

	bool IsInt;
	int64_t IntValue;
	long double LongDoubleValue;
};
