#ifndef LLPARSER_SYMBOL_H
#define LLPARSER_SYMBOL_H

class Symbol: public std::string
{
public:
	Symbol() = default;
	bool IsTerminal() const;
	bool IsNonterminal() const;
	std::string GetValue() const;

	static char const TERMINAL_LEFT_BORDER;
	static char const TERMINAL_RIGHT_BORDER;

	static char const NONTERMINAL_LEFT_BORDER;
	static char const NONTERMINAL_RIGHT_BORDER;
};

#endif
