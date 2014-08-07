#pragma once

struct Complex
{
	double re;
	double im;

	Complex operator+(const Complex other) const;
	
	Complex operator-(const Complex other) const;

	Complex operator*(const Complex other) const;

	bool operator==(const Complex other) const;
	
	double abs() const;
};