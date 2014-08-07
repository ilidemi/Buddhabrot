#include "complex.h"

Complex Complex::operator+(const Complex other) const
{
	return{ re + other.re, im + other.im };
}

Complex Complex::operator-(const Complex other) const
{
	return { re - other.re, im - other.im };
}

Complex Complex::operator*(const Complex other) const
{
	return{ re * other.re - im * other.im, re * other.im + im * other.re };
}

bool Complex::operator==(const Complex other) const
{
	return re == other.re && im == other.im;
}

double Complex::abs() const
{
	return re*re + im*im;
}