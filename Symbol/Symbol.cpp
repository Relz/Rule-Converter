#include "Symbol.h"

Symbol::Symbol(std::string const & string) : basic_string(string)
{
}

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

bool Symbol::IsValueEquals(std::string const & other) const
{
	if (this->size() - 2 != other.size())
	{
		return false;
	}
	for (size_t i = 1; i < this->size() - 1; ++i)
	{
		if (this->at(i) != other.at(i - 1))
		{
			return false;
		}
	}
	return true;
}

bool Symbol::operator==(Symbol const & other) const
{
	return this->compare(other) == 0;
}

char const Symbol::TERMINAL_LEFT_BORDER = '[';
char const Symbol::TERMINAL_RIGHT_BORDER = ']';

char const Symbol::NONTERMINAL_LEFT_BORDER = '<';
char const Symbol::NONTERMINAL_RIGHT_BORDER = '>';

char const Symbol::ACTION_NAME_LEFT_BORDER = '{';
char const Symbol::ACTION_NAME_RIGHT_BORDER = '}';

void Symbol::CreateTerminal(std::string const & value, Symbol & terminal)
{
	terminal.clear();
	terminal += TERMINAL_LEFT_BORDER;
	terminal += value;
	terminal += TERMINAL_RIGHT_BORDER;
}

void Symbol::CreateNonterminal(std::string const & value, Symbol & nonterminal)
{
	nonterminal.clear();
	nonterminal += NONTERMINAL_LEFT_BORDER;
	nonterminal += value;
	nonterminal += NONTERMINAL_RIGHT_BORDER;
}

void Symbol::CreateActionName(std::string const & value, Symbol & actionName)
{
	actionName.clear();
	actionName += ACTION_NAME_LEFT_BORDER;
	actionName += value;
	actionName += ACTION_NAME_RIGHT_BORDER;
}
