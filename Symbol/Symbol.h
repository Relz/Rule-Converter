#ifndef LLPARSER_SYMBOL_H
#define LLPARSER_SYMBOL_H

#include <string>

class Symbol: public std::string
{
public:
	Symbol() = default;
	Symbol(std::string const & value);
	bool IsTerminal() const;
	bool IsNonterminal() const;
	std::string GetValue() const;
	bool IsValueEquals(std::string const & other) const;

	bool operator==(Symbol const & other) const;

	static char const TERMINAL_LEFT_BORDER;
	static char const TERMINAL_RIGHT_BORDER;

	static char const NONTERMINAL_LEFT_BORDER;
	static char const NONTERMINAL_RIGHT_BORDER;

	static char const ACTION_NAME_LEFT_BORDER;
	static char const ACTION_NAME_RIGHT_BORDER;

	static void CreateTerminal(std::string const & value, Symbol & terminal);
	static void CreateNonterminal(std::string const & value, Symbol & nonterminal);
};

namespace std
{
	template<>
	struct hash<Symbol>
	{
		std::size_t operator()(const Symbol & symbol) const
		{
			return hash<std::string>()(symbol);
		}
	};
}

#endif
