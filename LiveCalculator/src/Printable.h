#pragma once

class Printable
{
public:
	virtual ~Printable() = default;
	virtual void Print() = 0;
};
