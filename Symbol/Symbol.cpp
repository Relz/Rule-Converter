#include <string>
#include "Symbol.h"

bool Symbol::IsTerminal() const
{
	return this->front() == TERMINAL_LEFT_BORDER;
}

bool Symbol::IsNonterminal() const
{
	return this->front() == NONTERMINAL_LEFT_BORDER;
}

std::string Symbol::GetValue() const
{
	return this->substr(1, this->size() - 2);
}

char const Symbol::TERMINAL_LEFT_BORDER = '[';
char const Symbol::TERMINAL_RIGHT_BORDER = ']';

char const Symbol::NONTERMINAL_LEFT_BORDER = '<';
char const Symbol::NONTERMINAL_RIGHT_BORDER = '>';
